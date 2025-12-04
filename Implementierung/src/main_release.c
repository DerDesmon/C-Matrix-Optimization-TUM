#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>
#include "matmul.h"
#include "matmul_caller.h"

#define OPTSTRING "V:B::a:b:o:h"
#define NUMBER_OF_VS 3

static void print_help()
{
    printf("Help (Release)\n");
    printf("-V <number> — Implementation: 0=auto, 1=SIMD, 2=no SIMD, 3=unsorted\n");
    printf("-B<number> — Benchmark repetitions (e.g., -B or -B5)\n");
    printf("-a <filename> — Input matrix A (ELLPACK)\n");
    printf("-b <filename> — Input matrix B (ELLPACK)\n");
    printf("-o <filename> — Output file (default: gen/matrix.txt)\n");
    printf("-h — Show help\n");
}

static bool parse_int(char *str, int *out)
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
    int V = 0;
    int B = 0;
    char *a = NULL;
    char *b = NULL;
    char *o = NULL;
    bool show_help = false;

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
        case '?':
            return EXIT_FAILURE;
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            return EXIT_FAILURE;
        }
    }

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

    if (a == NULL || b == NULL)
    {
        fprintf(stderr, "a or b was not set, use -h for help\n");
        return EXIT_FAILURE;
    }

    if (o == NULL)
    {
        printf("o was not set, using default value of `gen/matrix.txt`\n");
        o = "gen/matrix.txt";
    }

    return call_matmul(a, b, o, B, matmul);
}
