#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.c"
#include "../src/benchmark.h"
#include "../include/ellpack.h"
#include "../src/matrix_utils.h"
#include "../src/matmul.h"
#include "../src/matmul_caller.h"

char base_path[] = "dev/inputs/dist/"; //when run from root (Implementierung)
char benchmark_multiplications[][2][30] = {
    {"dense_regular_square.txt", "dense_regular_square.txt"},
    {"sparse_regular_square.txt", "sparse_regular_square.txt"},
    {"sparse_tall.txt", "single_column_sparse.txt"},
    {"dense_tall.txt", "single_column_sparse.txt"},
    {"dense_tall.txt", "single_column_dense.txt"},
    {"single_row_sparse.txt", "single_column_sparse.txt"},
    {"single_row_dense.txt", "single_column_dense.txt"},
    {"10x10_sparse.txt", "10x10_sparse.txt"},
    {"large_single_column_dense.txt", "large_square_dense.txt"},
    {"large_single_column_sparse.txt", "large_square_dense.txt"},
    {"xl_square_sparse.txt", "xl_square_sparse.txt"},
};
int multiplications_len = sizeof(benchmark_multiplications) / sizeof(benchmark_multiplications[0]);

int benchmark_main(matmul_func matmul) {
    for (int i = 0; i < multiplications_len; i++)
    {
        char* full_path_b = NULL; //for cleanup
        char* filename_a = benchmark_multiplications[i][0];
        char* filename_b = benchmark_multiplications[i][1];
        char* full_path_a = malloc_concat_strings(base_path, filename_a);
        full_path_b = malloc_concat_strings(base_path, filename_b);
        if(full_path_a == NULL || full_path_b == NULL) {
            fprintf(stderr, "Could not allocate memory for full path\n");
            free(full_path_a);
            free(full_path_b);
            //not going to cleanup here, as the program will exit anyway
            return EXIT_FAILURE;
        }
        printf("Benchmarking `%s` * `%s`:\n", filename_a, filename_b);
        int result = call_matmul(full_path_a, full_path_b, NULL, 3, matmul);
        printf("----\n");
        if(result == EXIT_FAILURE) {
            fprintf(stderr, "Benchmark failed on iteration %d\n", i);
            return EXIT_FAILURE;
        }
        free(full_path_a);
        free(full_path_b);   
    }
    return EXIT_SUCCESS;
}