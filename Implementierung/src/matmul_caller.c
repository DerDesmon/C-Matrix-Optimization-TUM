#include "../include/ellpack.h"
#include <stdlib.h>
#include "matrix_utils.h"
#include "io.h"
#include "benchmark.h"


int call_matmul(const char* filename_a, const char* filename_b, const char* output_file, int benchmark_iterations, matmul_func matmul) {
    ELLPACKMatrix mat_a = get_empty_ellpackmatrix();
    if (read_ellpack_matrix(&mat_a, filename_a))
    {
        clean_matrix_data(&mat_a);
        return EXIT_FAILURE;
    }

    //init for cleanup, so that we can always use the cleanup label
    result_mat result_matrix = {
        .cols = NULL,
        .cols_len = 0
    };

    ELLPACKMatrix mat_b = get_empty_ellpackmatrix();;
    if (read_ellpack_matrix(&mat_b, filename_b)) goto cleanup_error;

    result_matrix = malloc_init_result_mat_from_ellpack(&mat_a, &mat_b);
    if (result_matrix.cols == NULL)
    {
        fprintf(stderr, "Could not allocate memory for initial result matrix\n");
        goto cleanup_error;
    }

    // MATMUL
    if(benchmark_iterations > 0) {
        run_benchmark(benchmark_iterations, &mat_a, &mat_b, &result_matrix, matmul);
    }
    else {
        matmul(&mat_a, &mat_b, &result_matrix);
    }

    if(result_matrix.cols == NULL) goto cleanup_error;

    //result_width can be read from result_matrix.cols -> save it
    uint64_t result_rows = mat_a.nr_rows; //these are actual rows not ellpack rows
    //already free this data, since it is not needed anymore
    clean_matrix_data(&mat_a);
    clean_matrix_data(&mat_b);

    // after matmul
    bool error_occured_in_write = false;
    if(output_file != NULL) {
        result_file *result = write_ellpack_matrix(output_file, &result_matrix, result_rows, result_matrix.cols_len, &error_occured_in_write);
        free(result);
        if (error_occured_in_write) goto cleanup_error;
    }

    bool error_occured = false;
    goto cleanup;
cleanup_error:
    error_occured = true;
cleanup:
    // ENDE muss egal ob fail oder nicht gefreeed werden
    free_result_mat(&result_matrix);
    clean_matrix_data(&mat_a);
    clean_matrix_data(&mat_b);
    return error_occured ? EXIT_FAILURE : EXIT_SUCCESS;
}
