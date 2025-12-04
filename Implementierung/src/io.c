#define _POSIX_C_SOURCE 200809L
#define BUFFER_SIZE 1024

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "../include/ellpack.h"
#include "io.h"
#include "matrix_utils.h"

#define expect_0(x) __builtin_expect(!!(x), 0) // for unlikely error handlers

/**
 * returns 1 if error occured, else 0
 * @param a Matrix to read into. Every pointer in this should be NULL
 * @param filename file in which we get the matrix
 * a->values is allocated -> freed if something failed with clean_matrix_data; else clean_matrix_data
 * ----needs to be called after the matmul (in main)
 * star_index is allocated (compares if stars in indices and values are at the same positions)-> freed if failed or successful with clean_file_data
 * a->nr_of_non_zeros_per_col -> freed if something failed with clean_matrix_data; else clean_matrix_data needs to be called after the matmul (in main)
 * a-indices -> freed if something failed with clean_matrix_data; else clean_matrix_data needs to be called after the matmul (in main)
 */
int read_ellpack_matrix(ELLPACKMatrix *a, const char *filename)
{
    uint8_t *initial_star_index = NULL;

    FILE *aptr = fopen(filename, "r");
    uint8_t *star_index = NULL;
    memset(a, 0, sizeof(ELLPACKMatrix)); // nullify the struct to prevent undefined behaviour in clean_error -> clean_matrix_data
    // for now, we assume that the indices are sorted
    a->sorted = true;

    if (aptr == NULL)
    {
        fprintf(stderr, "opening %s failed\n", filename);
        goto clean_error;
    }
    //* 1. line: <#rows>,<#cols>,<#Ellpackrows>
    char first_line[100];

    if (fgets(first_line, 98, aptr) == NULL)
    {
        fprintf(stderr, "reading the first line of %s failed\n", filename);
        goto clean_error;
    }
    if (strchr(first_line, '-') != NULL) // if minus in first line -> get rid of neg dimensions
    {
        fprintf(stderr, "%s contained negative dimension in the first line!\n", filename);
        goto clean_error;
    }
    if (sscanf(first_line, "%" SCNu64 ",%" SCNu64 ",%" SCNu64, &a->nr_rows, &a->nr_cols, &a->nr_ellpack_elts) != 3)
    {
        fprintf(stderr, "first line of matrix `%s` had the wrong format!\n", filename);
        goto clean_error;
    };

    //* 2. line: values (-> only keep the non star values)
    size_t max_nr_of_values = a->nr_ellpack_elts * a->nr_cols;

    // allocating the space for the ellpack arrays
    a->values = malloc(max_nr_of_values * sizeof(float));
    if ((a->values) == NULL)
    {
        fprintf(stderr, "allocating memory for the values array of `%s` failed\n", filename);
        goto clean_error;
    }
    a->nr_of_non_zeros_per_col = malloc(a->nr_cols * sizeof(uint64_t));
    if (a->nr_of_non_zeros_per_col == NULL)
    {
        fprintf(stderr, "allocating memory for the nr_of_non_zeros_per_col array of `%s` failed\n", filename);
        goto clean_error;
    }

    // calloc -> everything is zero -> no * found so far
    if ((star_index = calloc(max_nr_of_values, sizeof(uint8_t))) == NULL)
    {
        fprintf(stderr, "allocating memory to compare the * position failed!\n");
        goto clean_error;
    }

    initial_star_index = star_index;

    char cur_val_buf[BUFFER_SIZE] = {0};
    size_t old_val_buf_size = 0;
    size_t end = 0;
    uint64_t total_elements = 0;
    uint64_t exp_total_elts = a->nr_cols * a->nr_ellpack_elts;

    uint8_t end_of_line = 0;
    a->total_non_zero_nr = 0;
    size_t non_zero_elt_in_col = 0;
    size_t elt_in_col = 0;
    uint64_t col_index = 0;

    while (!end_of_line)
    {
        // get new values
        if (!fgets(cur_val_buf + old_val_buf_size, BUFFER_SIZE - old_val_buf_size, aptr))
        {
            if (!strlen(cur_val_buf))
                break;
        }

        // define where to stop in the current buffer
        char *breakpoint = strchr(cur_val_buf, '\n');
        if (breakpoint)
        { // if the line ends somewhere in this buffer -> stop then after reading the values
            end = breakpoint - cur_val_buf;
            *breakpoint = '\0';
            end_of_line = 1;
        }
        else
        {
            breakpoint = strrchr(cur_val_buf, ',');
            if (breakpoint)
            { // if no newline is in the current buffer, set the end to the last ,
                end = breakpoint - cur_val_buf;
            }
            else
            {
                end = strlen(cur_val_buf);
            }
        }
        char *values_left = cur_val_buf;

        // parse floats & count values
        while (values_left < breakpoint && total_elements < max_nr_of_values)
        {
            char *values = strtok_r(values_left, ",", &values_left); // get everything upto the first ','
            if (values)                                              // !=NULL
            {
                if (!strcmp(values, "*") || !strcmp(values, " *") || !strcmp(values, " * ") || !strcmp(values, "* "))
                { // Zero_elt
                    star_index[total_elements] = 1;
                    elt_in_col++;
                    total_elements++;
                }
                else
                {
                    char *endptr = NULL; // just to check if float could be matched
                    errno = 0;
                    float value = strtof(values, &endptr);
                    if (expect_0(values == endptr || errno != 0))
                    { // nothing could be matched
                        if (errno == ERANGE)
                            fprintf(stderr, "(a value in %s cannot be represented as a float): ", filename);
                        fprintf(stderr, "error when trying to convert float from string in %s!\n", filename);
                        goto clean_error;
                    }
                    else // will also be writing 0.0 values since otherwise the indices would not match
                    {    // found valid value
                        if (expect_0(isnan(value) || isinf(value)))
                            printf("warning: found %f in %s\n", value, filename);
                        a->values[a->total_non_zero_nr] = value;
                        a->total_non_zero_nr++;
                        non_zero_elt_in_col++;
                    }
                    elt_in_col++;
                    total_elements++;
                }
                if (elt_in_col == a->nr_ellpack_elts)
                {
                    // next col
                    if (col_index < a->nr_cols)
                    {
                        a->nr_of_non_zeros_per_col[col_index] = non_zero_elt_in_col;
                        elt_in_col = 0;
                        non_zero_elt_in_col = 0;
                        col_index++;
                    }
                    else
                    {
                        fprintf(stderr, "error: too many values!\n");
                        goto clean_error;
                    }
                }
            }
            else
            {
                break; // current buffer is done
            }
        }

        /* shift everthing which was behind the last comma to the beginning -> makes sure that only complete values are parsed
        if half value was parsed -> will be completed with next buffer filling */
        if (!end_of_line)
        {
            // get the size of bytes that were behind the last comma in the bufffer
            old_val_buf_size = strlen(cur_val_buf + end + 1);
            // move stuff from behind the last comma to the beginning of the buffer
            memmove(cur_val_buf, cur_val_buf + end + 1, old_val_buf_size);
            // make sure that next buffer filling starts behind half parsed values
            cur_val_buf[old_val_buf_size] = '\0';
        }
    }
    if (expect_0(total_elements != exp_total_elts))
    {
        fprintf(stderr, " `%s` had %" PRIu64 " instead of %" PRIu64 " values \n", filename, total_elements, exp_total_elts);
        goto clean_error;
    }
    /*get 3.LINE*/
    if (!(a->indices = calloc(max_nr_of_values, sizeof(uint64_t))))
    {
        fprintf(stderr, "allocating memory for the indices array of a failed\n");
        goto clean_error;
    }

    uint8_t end_of_line_3 = 0;
    char cur_ind_buf[BUFFER_SIZE] = {0};
    size_t old_ind_buf_size = 0;
    uint64_t total_indices = 0;
    elt_in_col = 0;
    size_t non_star_index = 0;
    while (!end_of_line_3)
    {
        // get new values
        if (!fgets(cur_ind_buf + old_ind_buf_size, BUFFER_SIZE - old_ind_buf_size, aptr))
        {
            if (!strlen(cur_ind_buf))
            {
                break;
            }
        }
        if (expect_0(strchr(cur_ind_buf, '-') != NULL))
        {
            fprintf(stderr, "negative indices are not valid!\n");
            goto clean_error;
        }

        // where to stop parsing in the current buffer
        char *breakpoint = strchr(cur_ind_buf, '\n');
        if (breakpoint)
        { // if the line ends somewhere in this buffer -> stop then
            end = breakpoint - cur_ind_buf;
            *breakpoint = '\0';
            end_of_line_3 = 1;
        }

        else if (strlen(cur_ind_buf) < BUFFER_SIZE - 1) // if filled cur_ind_buf = max 1023
        {
            end_of_line_3 = 1;
            end = strlen(cur_ind_buf);
            breakpoint = strchr(cur_ind_buf, '\0');
        }
        else
        {
            breakpoint = strrchr(cur_ind_buf, ',');
            if (breakpoint)
            { // if no newline is in the current buffer, set the end to the last ','
                end = breakpoint - cur_ind_buf;
            }
            else
            {
                end = strlen(cur_ind_buf);
            }
        }

        char *indices_left = cur_ind_buf;
        uint64_t last_indice = 0;

        // parse indices
        while (indices_left < breakpoint && total_indices < max_nr_of_values)
        {
            char *indices = strtok_r(indices_left, ",", &indices_left);
            // everything upto the first , -> should be first value
            if (indices)
            {
                if (!strcmp(indices, "*") || !strcmp(indices, " *") || !strcmp(indices, " * ") || !strcmp(indices, "* "))
                {                                                 // Zero_elt
                    if (expect_0(star_index[total_indices] != 1)) // check if stars are on the same indices as for values
                    {
                        fprintf(stderr, "found a index star, but no corresponding value star!\n");
                        goto clean_error;
                    }
                    last_indice++;
                }
                else
                {
                    char *endptr = NULL; // just to check if float could be matched
                    uint64_t indice = strtoull(indices, &endptr, 10);
                    if (expect_0(indices == endptr))
                    { // nothing could be matched
                        fprintf(stderr, "error when trying to convert uint64_t from string!\n");
                        goto clean_error;
                    }
                    else if (expect_0(indice >= a->nr_rows))
                    {
                        fprintf(stderr, "invalid index found in  %s (i > number of cols )!\n", filename);
                        goto clean_error;
                    }
                    else if (expect_0(indice == last_indice && elt_in_col != 0))
                    {
                        fprintf(stderr, "index appeard twice in a col!\n");
                        goto clean_error;
                    }
                    else if (expect_0(star_index[total_indices] == 1))
                    {
                        fprintf(stderr, "found an index value, but value was a star!\n");
                        goto clean_error;
                    }
                    else
                    { // found good indice
                        if (indice < last_indice)
                        {
                            fprintf(stderr, "indices are not sorted: %" PRIu64 ", %" PRIu64 "!\n", last_indice, indice);
                            a->sorted = false;
                        }
                        a->indices[non_star_index] = indice;
                        non_star_index++;
                        last_indice = indice;
                    }
                }
                total_indices++;
                elt_in_col++;
                if (elt_in_col == a->nr_ellpack_elts)
                {
                    elt_in_col = 0;
                    last_indice = 0;
                }
            }
            else
            {
                break; // current buffer is done
            }
        }
        /* shift everthing which was behind the last comma to the beginning -> makes sure that only complete indices are parsed
       if half indice was parsed -> will be completed with next buffer filling */
        if (!end_of_line_3)
        {
            // get the size of what was behind the last comma in the bufffer
            old_ind_buf_size = strlen(cur_ind_buf + end + 1);
            // move value from behind the last comma to the beginning of the next buffer
            memmove(cur_ind_buf, cur_ind_buf + end + 1, old_ind_buf_size);
            // make sure that filling next buffer starts behind half parsed indices
            cur_ind_buf[old_ind_buf_size] = '\0';
        }
    }

    if (0.9 * max_nr_of_values > a->total_non_zero_nr)
    { // >= 10% of allocated space is not used -> resize

        float *new_values = NULL;
        if (expect_0(!(new_values = realloc(a->values, a->total_non_zero_nr * sizeof(float)))))
        {
            fprintf(stderr, "reallocating memory for the values array of `%s` failed\n", filename);
            goto clean_error;
        }
        else
        {
            a->values = new_values;
        }

        uint64_t *new_indices = NULL;
        if (expect_0(!(new_indices = realloc(a->indices, a->total_non_zero_nr * sizeof(uint64_t)))))
        {
            fprintf(stderr, "reallocating memory for the indices array of `%s` failed\n", filename);
            ;
            goto clean_error;
        }
        else
        {
            a->indices = new_indices;
        }
    }
    goto clean_all;

clean_error:
    clean_file_data(initial_star_index, aptr);
    clean_matrix_data(a);
    return 1;

clean_all:
    clean_file_data(initial_star_index, aptr);
    return 0;
}
/**
 * @param filename where the result will be printed into -> file is closed before return
 * @param matrix the matrix to be printed out
 * @param rows of result
 * @param cols of result
 * @param error_occurred_in_write -> to check if valid result or an error occured after the funciton is done
 * @return result has to be freed at the end of main -> free(result)
 */
result_file *write_ellpack_matrix(const char *filename, result_mat *matrix, uint64_t rows, uint64_t cols, bool *error_occurred_in_write)
{
    *error_occurred_in_write = false;
    result_file *result = malloc(sizeof(result_file));
    if (!result)
    {
        fprintf(stderr, "could not allocate space for the result!\n");
        *error_occurred_in_write = true;
        goto clean_up;
    }
    result->filename = filename;
    result->output = fopen(filename, "w");
    if (!result->output || !matrix)
    {
        fprintf(stderr, "invalid result filename or result matrix!\n");
        *error_occurred_in_write = true;
        goto clean_up;
    }
    unsigned int ellpack_col_len = get_longest_col(matrix);
    unsigned int nr_of_cols = matrix->cols_len;
    // first line
    if (0 > fprintf(result->output, "%" PRIu64 ",%" PRIu64 ",%u\n", rows, cols, ellpack_col_len))
    {
        fprintf(stderr, "invaild result filename or result matrix!\n");
        *error_occurred_in_write = true;
        goto clean_up;
    }
    if (!nr_of_cols)
    {
        fprintf(stderr, "matrix has no cols!\n"); // nothing more to do
        *error_occurred_in_write = true;
        goto clean_up;
    }
    // second line + include the stars
    // print each col
    // last col extra -> no comma in the end
    for (size_t col_nr = 0; col_nr < nr_of_cols - 1; col_nr++)
    { // enough space for col, ',' and \n & \0 (16 chars per float max + comma)
        char *col_buffer = malloc(ellpack_col_len * 18 + 2);
        if (!col_buffer)
        {
            fprintf(stderr, "allocating memory for a Buffer to print the result values to file failed!\n");
            *error_occurred_in_write = true;
            goto clean_up;
        }
        result_col *cur_col = &matrix->cols[col_nr];
        char *buf_ptr = col_buffer;
        unsigned int height = cur_col->used_height;
        // go through col
        for (size_t row_in_col = 0; row_in_col < height; row_in_col++)
        { // are all values != 0?
            buf_ptr += sprintf(buf_ptr, "%e,", cur_col->values[row_in_col]);
        }
        size_t nr_stars = ellpack_col_len - height;
        // include stars
        while (nr_stars > 0)
        {
            buf_ptr += sprintf(buf_ptr, "*,");
            nr_stars--;
        }
        int buf_size = buf_ptr - col_buffer;
        if (buf_size > 0)
            fprintf(result->output, "%*s", buf_size, col_buffer);

        free(col_buffer);
    }
    // print last col without comma in the end
    char *col_buffer = calloc(ellpack_col_len * 18 + 2, sizeof(char));
    if (!col_buffer)
    {
        fprintf(stderr, "allocating memory for a Buffer to print the result values to file failed!\n");
        *error_occurred_in_write = true;
        goto clean_up;
    }
    result_col *cur_col = &matrix->cols[nr_of_cols - 1];
    char *buf_ptr = col_buffer;
    unsigned int height = cur_col->used_height;
    if (height)
    {
        // go through col
        for (size_t row_in_col = 0; row_in_col < height - 1; row_in_col++)
        { // are all values != 0?
            // sprintf: nr of chars printed, without counting nullbyte is returned, but adds a nullbyte -> overwritten
            buf_ptr += sprintf(buf_ptr, "%e,", cur_col->values[row_in_col]);
        }
        buf_ptr += sprintf(buf_ptr, "%e", cur_col->values[height - 1]);
    }
    size_t nr_stars = ellpack_col_len - height;
    if (nr_stars > 0 && height > 0)
    {
        buf_ptr += sprintf(buf_ptr, ",");
    }
    // include stars
    while (nr_stars > 0)
    {
        buf_ptr += sprintf(buf_ptr, nr_stars == 1 ? "*" : "*,");
        nr_stars--;
    }
    // values are done -> print them to the output file
    buf_ptr += sprintf(buf_ptr, "\n");
    int buf_size = buf_ptr - col_buffer;
    if (buf_size > 0)
        fprintf(result->output, "%*s", buf_size, col_buffer);

    free(col_buffer);

    // third line
    // same for indices
    // for each col, last col extra -> no comma in the end
    for (size_t col_nr = 0; col_nr < nr_of_cols - 1; col_nr++)
    { // enough space for col, ',' and \n & \0 (20 chars per uint_64t max + comma)
        char *col_buffer_ind = malloc(ellpack_col_len * 25 + 2);
        if (!col_buffer_ind)
        {
            fprintf(stderr, "allocating memory for a Buffer to print the result indices to file failed!\n");
            *error_occurred_in_write = true;
            goto clean_up;
        }
        result_col *cur_col_ind = &matrix->cols[col_nr];
        char *buf_ptr_ind = col_buffer_ind;
        unsigned int height_ind = cur_col_ind->used_height;

        // go through col
        for (size_t row_in_col = 0; row_in_col < height_ind; row_in_col++)
        {
            buf_ptr_ind += sprintf(buf_ptr_ind, "%" PRIu64 ",", cur_col_ind->indices[row_in_col]);
        }
        size_t nr_stars_ind = ellpack_col_len - height_ind;
        // include stars
        while (nr_stars_ind > 0)
        {
            buf_ptr_ind += sprintf(buf_ptr_ind, "*,");
            nr_stars_ind--;
        }
        int buf_size_ind = buf_ptr_ind - col_buffer_ind;
        if (buf_size_ind > 0)
            fprintf(result->output, "%*s", buf_size_ind, col_buffer_ind);

        free(col_buffer_ind);
    }
    // print last indices without comma
    char *col_buffer_ind = malloc(ellpack_col_len * 25 + 2);
    if (!col_buffer_ind)
    {
        fprintf(stderr, "allocating memory for a Buffer to print the result indices to file failed!\n");
        *error_occurred_in_write = true;
        goto clean_up;
    }
    result_col *cur_col_ind = &matrix->cols[nr_of_cols - 1];
    char *buf_ptr_ind = col_buffer_ind;
    unsigned int height_ind = cur_col_ind->used_height;
    if (height_ind)
    {
        // go through col
        for (size_t row_in_col = 0; row_in_col < height_ind - 1; row_in_col++)
        {
            buf_ptr_ind += sprintf(buf_ptr_ind, "%" PRIu64 ",", cur_col_ind->indices[row_in_col]);
        }
        buf_ptr_ind += sprintf(buf_ptr_ind, "%" PRIu64, cur_col_ind->indices[height_ind - 1]);
    }
    size_t nr_stars_ind = ellpack_col_len - height_ind;
    // include stars
    if (nr_stars_ind > 0 && height > 0)
    {
        buf_ptr_ind += sprintf(buf_ptr_ind, ",");
    }
    while (nr_stars_ind > 0)
    {
        buf_ptr_ind += sprintf(buf_ptr_ind, nr_stars_ind == 1 ? "*" : "*,");
        nr_stars_ind--;
    }
    int buf_size_ind = buf_ptr_ind - col_buffer_ind;
    if (buf_size_ind > 0)
        fprintf(result->output, "%*s", buf_size_ind, col_buffer_ind);

    free(col_buffer_ind);

clean_up:
    if (result != NULL && result->output != NULL)
    {
        fclose(result->output);
    }
    return result;
}