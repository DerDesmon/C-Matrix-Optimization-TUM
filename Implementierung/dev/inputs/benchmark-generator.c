#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include "benchmark-generator.h"
#include "../helpers.c"

//root/dev/inputs/dist file should be created

//file should be run from root/bin
#define DISTPATH "dev/inputs/dist/"
#define MATRIX_SIZE 1000
#define DONT_CARE '*'


void write_index(FILE* f_ptr, uint64_t value, bool is_final) {
    bool is_dont_care = value == UINT64_MAX;
    if(is_dont_care) fprintf(f_ptr, "%c", DONT_CARE);
    else fprintf(f_ptr, "%" PRIu64, value);
    if(!is_final) fprintf(f_ptr, ",");
}

void write_value(FILE* f_ptr, float value, bool is_final) {
    bool is_nan = value != value;
    if(is_nan) fprintf(f_ptr, "%c", DONT_CARE);
    else fprintf(f_ptr, "%.1f", value);
    if(!is_final) fprintf(f_ptr, ",");
}

int generate_matrix_benchmark(generator_config config, unsigned int seed)
{
    /* initial calculations */
    uint64_t MAX_DIMENSION = UINT64_MAX - 1; // because 2^64-1 is used as don't care index
    if (config.height > MAX_DIMENSION || config.width > MAX_DIMENSION || config.col_value_count == 0 || config.filling == 0)
    {
        fprintf(stderr, "Invalid config\n");
        return EXIT_FAILURE;
    }

    uint64_t fullest_col_number = config.col_value_count;
    uint64_t emptiest_col_number = (config.filling * fullest_col_number) / 255; // difference between fullest and emptiest column
    uint64_t col_delta = fullest_col_number - emptiest_col_number;

    __uint128_t number_of_values = config.col_value_count * config.width;

    /* malloc */
    // set to 2^64-2 if is a don't care
    uint64_t *indices = malloc(number_of_values * sizeof(uint64_t));
    if (indices == NULL){
        fprintf(stderr, "Failed ot malloc indices\n");
        return EXIT_FAILURE;
    }
    // set to NaN for don't cares
    float *values = malloc(number_of_values * sizeof(float));
    if (values == NULL)
    {
        fprintf(stderr, "Failed ot malloc values\n");
        free(indices);
        return EXIT_FAILURE;
    }

    char *filepath = malloc_concat_strings(DISTPATH, config.filename);
    if (filepath == NULL)
    {
        fprintf(stderr, "Failed to malloc filepath\n");
        free(indices);
        free(values);
        return EXIT_FAILURE;
    }

    FILE *fileptr = fopen(filepath, "w");
    if(fileptr == NULL) {
        fprintf(stderr, "Failed to open file with path %s. Please ensure <root>/dev/inputs/dist exists and you are calling the binary from <root> \n", filepath);
        free(indices);
        free(values);
        free(filepath);
        return EXIT_FAILURE;
    }

    /* seed with values */

    // fill indices:
    // for each column:
    // determine how many values are filled (number between emptiest_col_number and fullest_col_number)
    // fill the valid indices
    // fill the rest with 2^64-1
    // then, fill a random column completely
    srand(seed);
    uint64_t column_to_fill = (config.width > 1) ? rand() % (config.width - 1) : 0;

    for (uint64_t col_i = 0; col_i < config.width; col_i++)
    {
        __uint128_t indices_idx = col_i * config.col_value_count;
        uint64_t filled_count = (rand() % (col_delta == 0 ? 1 : col_delta)) + emptiest_col_number;
        if (col_i == column_to_fill) filled_count = fullest_col_number;
        if(filled_count > fullest_col_number) filled_count = fullest_col_number;
        uint64_t step_size = config.height / (filled_count == 0 ? 1 : filled_count); // if filled_count == 0, then no values are filled
        for (uint64_t filled_i = 0; filled_i < filled_count; filled_i++)
        {
            uint64_t offset = rand() % step_size;
            indices[indices_idx] = filled_i * step_size + offset;
            indices_idx++;
        }
        for (uint64_t filled_i = filled_count; filled_i < config.col_value_count; filled_i++)
        {
            indices[indices_idx] = UINT64_MAX;
            indices_idx++;
        }
    }

    // fill values:
    // for each (array) index in (matrix) indices
    // fill corresponding index in values with random number (float or a rounded float depending on config.only_ints)
    // if indices[index] is 2^64-2, then fill with NaN
    for (__uint128_t i = 0; i < number_of_values; i++)
    {
        if (indices[i] == UINT64_MAX)
        {
            values[i] = (0.0 / 0.0); // set to NaN
        }
        else
        {
            float random_int = (float)rand();
            float random_float = (random_int / (float) RAND_MAX) * 1000.0; // float between 0 and 1000
            values[i] = config.only_ints ? random_int : random_float;
        }
    }

    /* write to file */
    //metadata
    fprintf(fileptr, "%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n", config.height, config.width, config.col_value_count);

    //values
    for (__uint128_t val_i = 0; val_i < (number_of_values - 1); val_i++)
    {
        write_value(fileptr, values[val_i], false);
    }
    //write last value
    write_value(fileptr, values[number_of_values - 1], true);
    fprintf(fileptr, "\n");


    //write indicies
    for (__uint128_t idx_i = 0; idx_i < (number_of_values - 1); idx_i++)
    {
        write_index(fileptr, indices[idx_i], false);
    }
    write_index(fileptr, indices[number_of_values - 1], true);
    

    /* cleanup */
    free(indices);
    free(values);
    free(filepath);
    fclose(fileptr);

    return EXIT_SUCCESS;
}

/* for context:
typedef struct
{
    char *filename;
    uint64_t height;          // number of rows
    uint64_t width;           // number of columns
    uint64_t col_value_count; // how many values the most full column contains
    uint8_t filling;          // defines the delta between most full and least full column (low value -> high delta)
    bool only_ints;
} generator_config;
*/
generator_config configs[] = {
    //square matrices of size MATRIX_SIZE
    {"sparse_regular_square.txt", MATRIX_SIZE, MATRIX_SIZE, 50, 100, false},
    {"dense_regular_square.txt", MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE, 255, false},
    {"dense_irregular_square.txt", MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE, 64, false},
    //tall matrices of width MATRIX_SIZE
    {"sparse_tall.txt", UINT64_MAX - 1, MATRIX_SIZE, 50, 255, false},
    {"dense_tall.txt", UINT64_MAX - 1, MATRIX_SIZE, MATRIX_SIZE/4, 255, false},
    //single row matrices (MATRIX_SIZE columns)
    {"single_row_sparse.txt", 1, MATRIX_SIZE, 1, 1, false},
    {"single_row_dense.txt", 1, MATRIX_SIZE, 1, 255, false},
    //single column matrices (MATRIX_SIZE rows)
    {"single_column_sparse.txt", MATRIX_SIZE, 1, 100, 1, false},
    {"single_column_dense.txt", MATRIX_SIZE, 1, MATRIX_SIZE/4, 255, false},
    //small matrices to test overhead
    {"10x10_sparse.txt", 10, 10, 4, 255, true},
    // xl_square_sparse.txt - MATRIX_SIZE*10 x MATRIX_SIZE*10
    {"xl_square_sparse.txt", MATRIX_SIZE*10, MATRIX_SIZE*10, 100, 1, false},
    // large_square_sparse.txt - MATRIX_SIZE*4 x MATRIX_SIZE*4
    {"large_square_sparse.txt", MATRIX_SIZE*4, MATRIX_SIZE*4, 100, 1, false},
    // large_square_dense.txt - MATRIX_SIZE*4 x MATRIX_SIZE*4
    {"large_square_dense.txt", MATRIX_SIZE*4, MATRIX_SIZE*4, MATRIX_SIZE*2, 255, false}, //is a lot of values
    {"large_single_column_dense.txt", MATRIX_SIZE*4, 1, MATRIX_SIZE*4, 255, false}, //is a lot of values
    {"large_single_column_sparse.txt", MATRIX_SIZE*4, 1, MATRIX_SIZE*4, 1, false}, //is a lot of values
    {"test_max_uint64.txt", UINT64_MAX - 1, 5, 5, 40, false},
    {"100x100_random.txt", 100, 100, 50, 100, false},
};
size_t configslen = sizeof(configs) / sizeof(configs[0]);

void generate_matrices_main() {
    bool err = false;

    printf("Generating benchmark matrices...\n");

    for (size_t i = 0; i < configslen; i++)
    {
        generator_config config = configs[i];
        int result = generate_matrix_benchmark(config, i);
        if(result != EXIT_SUCCESS) {
            fprintf(stderr, "Failed to generate matrix with filename %s\n", config.filename);
            err = true;
        }
    }
    
    if(err == false) printf("Success: generated all benchmark matrices\n");
}