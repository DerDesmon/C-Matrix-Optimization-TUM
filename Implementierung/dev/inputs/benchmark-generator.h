#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    char *filename;
    uint64_t height;          // number of rows
    uint64_t width;           // number of columns
    uint64_t col_value_count; // how many values the most full column contains
    uint8_t filling;          // defines the delta between most full and least full column (low value -> high delta)
    bool only_ints;
} generator_config;


int generate_matrix_benchmark(generator_config config, unsigned int seed);
void generate_matrices_main(); //generates all needed src files