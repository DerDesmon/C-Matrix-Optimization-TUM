#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "../include/ellpack.h"
#include "matrix_utils.h"
#include "matmul.h"
#include <unistd.h>

//see docs in benchmark.h
void run_benchmark(int n_times, const ELLPACKMatrix* data_a, const ELLPACKMatrix* data_b, result_mat* result_matrix, matmul_func matmul) {
    struct timespec result;
    result.tv_sec = 0;
    result.tv_nsec = 0;

    result_mat tmp;

    for (int i = 0; i < n_times; i++)
    {
        tmp = malloc_init_result_mat_from_ellpack(data_a, data_b);
        if (tmp.cols == NULL) {
            fprintf(stderr, "Could not allocate memory for temporary result matrix in benchmark\n");
            free_result_mat(result_matrix); // signal that the matmul was invalid
            return;
        }
        struct timespec start;
        struct timespec end;
        //avoid compiler optimizations
        __asm__ __volatile__ ("" : : : "memory");
        //start
        clock_gettime(CLOCK_MONOTONIC, &start);

        matmul(data_a, data_b, i == 0 ? result_matrix : &tmp);

        clock_gettime(CLOCK_MONOTONIC, &end);
        //end
        result.tv_nsec += (end.tv_nsec - start.tv_nsec);
        result.tv_sec += (end.tv_sec - start.tv_sec);
        if (result.tv_nsec >= 1000000000) {
            result.tv_nsec -= 1000000000;
            result.tv_sec += 1;
        }

        if(result_matrix->cols == NULL) {
            fprintf(stderr, "Warning: benchmark failed on iteration %d of %d, benchmark results are potentially invalid\n", (i + 1), n_times);
        }
        free_result_mat(&tmp);

        if(i != n_times - 1) {
            sleep(1); // sleep for 1 second to avoid overheating
        }
    }

    double total_time = result.tv_sec + (result.tv_nsec * 1e-9); 
    double average_time = total_time / n_times;

    printf("Benchmark took an average of %f seconds per iteration (total: %f)\n", average_time, total_time);
}
