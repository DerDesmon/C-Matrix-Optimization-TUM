    #ifndef TEST_H
    #define TEST_H

    #include <stdint.h>
    #include "../src/matrix_utils.h"

    typedef struct {
        uint64_t **indices;
        float **values;
        uint64_t rows;
        uint64_t cols;
        uint64_t max_values_per_col;
    } ellpack_matrix;

    void read_file(const char *filename, ellpack_matrix *matrix);
    int compare_ellpack_matrices(const ellpack_matrix *a, const ellpack_matrix *b);
    int test_main(matmul_func matmul);

    #endif // TEST_H
