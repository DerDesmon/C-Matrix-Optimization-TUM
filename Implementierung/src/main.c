#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include "../include/ellpack.h"
#include "io.h"
#include "matrix_utils.h"
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>
#include "matmul.h"
#include "benchmark.h"
#include "matmul_caller.h"
#include "../dev/test.h"
#include "../dev/benchmark_dev.h"


#define OPTSTRING "V:B::a:b:o:htp"
#define NUMBER_OF_VS 3 // ranging from 0 to <NUMBER_OF_VS>

void print_help();

bool parse_int(char *str, int *out)
{
    char *endptr;
    long result = strtol(str, &endptr, 10);
    if (result > INT_MAX || result < INT_MIN)
    {
        return false;
    }
    *out = (int)result;
    return *endptr == '\0';
}

int main(int argc, char **argv)
{
    int V = 0;              // which implementation to use -> is used to determine the function
    int B = 0;              // how many benchmark iterations to run (if 0, benchmarking is disabled)
    char *a = NULL;         // file path to matrix a
    char *b = NULL;         // file path to matrix b
    char *o = NULL;         // file path to result
    bool show_help = false; // if this is set, all other options should be ignored
    bool run_tests = false;
    bool run_benchmarks = false;

    int opt;
    int option_idx = 0;
    struct option long_opts[] = {
        {.name = "help", .has_arg = no_argument, .flag = 0, .val = 'h'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, OPTSTRING, long_opts, &option_idx)) != -1)
    {
        switch (opt)
        {
        case 'V':
            if (!parse_int(optarg, &V))
            {
                fprintf(stderr, "V could not be parsed as an integer\n");
                return EXIT_FAILURE;
            }
            if (V >= NUMBER_OF_VS)
            {
                fprintf(stderr, "V is greater than the number of implementations, maximum can be %d\n", NUMBER_OF_VS);
                return EXIT_FAILURE;
            }
            break;
        case 'B':
            B = 1;
            if (optarg != NULL && !parse_int(optarg, &B))
            {
                fprintf(stderr, "B could not be parsed as an integer\n");
                return EXIT_FAILURE;
            }
            if (B <= 0)
            {
                fprintf(stderr, "B must be >= 0 if it is set\n");
                return EXIT_FAILURE;
            }
            break;
        case 'a':
            a = optarg;
            break;
        case 'b':
            b = optarg;
            break;
        case 'o':
            o = optarg;
            break;
        case 'h':
            show_help = true;
            break;
        case 't':
            run_tests = true;
            break;
        case 'p':
            run_benchmarks = true;
            break;
        case '?':
            return EXIT_FAILURE;
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            return EXIT_FAILURE;
        }
    }

    // if h is set, print help and exit
    if (show_help)
    {
        print_help();
        return EXIT_SUCCESS;
    }

    
    matmul_func matmul = NULL;
    switch (V)
    {
    case 0:
        matmul = (matmul_func) matr_mult_ellpack;
        break;
    case 1: 
        matmul = (matmul_func) matr_mult_ellpack_main_simd;
        break;
    case 2: 
        matmul = (matmul_func) matr_mult_ellpack_main_no_simd;
        break;
    case 3: 
        matmul = (matmul_func) matr_mult_ellpack_unsorted;
        break;
    default:
        fprintf(stderr, "V is not defined for this value.\n");
        return EXIT_FAILURE;
    }

    if(run_tests) {
        return test_main(matmul);
    }

    if(run_benchmarks) {
        return benchmark_main(matmul);
    }

    // if a is NULL or b is NULL or o is NULL then fail
    if (a == NULL || b == NULL)
    {
        fprintf(stderr, "a, b or o was not set, use -h for help\n");
        return EXIT_FAILURE;
    }

    if(o == NULL) {
        printf("o was not set, using default value of `gen/matrix.txt`\n");
        o = "gen/matrix.txt";
    }
    /* main program, all options can be assumed as set & valid */

    return call_matmul(a, b, o, B, matmul);
}


void print_help()
{
    printf("Help Message\n");
    printf("-V <number> — The implementation to be used. If set to 0 or if the option is omitted, the main implementation is chosen\n");
    printf("\t0: main implementation, chooses V1 if matrix is sorted\n");
    printf("\t1: main implementation with simd dot product\n");
    printf("\t2: main implementation without simd (is useful for small or very sparse matrices)\n");
    printf("\t3: unsorted indices implementation (much much slower)\n");
    printf("-B<number> — If set, the runtime of the specified implementation is measured and output. The optional argument of this option specifies the number of repetitions of the function call, e.g. -B or -B5 (not set -> one repetition)\n");
    printf("-a <filename> — Input file containing matrix A\n");
    printf("-b <filename> — Input file containing matrix B\n");
    printf("-o <filename> — Output file. By default, is set to gen/matrix.txt\n");
    printf("-h|--help — A description of all program options and usage examples are output, and the program then exits\n");
    printf("The following two options are automated test/benchmarks that can be run from the CLI, but are not part of the required specification. For them to work, files need to be generated with a script. Before using them, execute make all-generate to set up the needed files.\n");
    printf("-t — Run tests (V can be defined, all other parameters are ignored)\n");
    printf("-p — Run performance benchmarks (V can be defined, all other parameters are ignored)\n");
}