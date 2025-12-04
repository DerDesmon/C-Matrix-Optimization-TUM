#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* malloc_concat_strings(char* str1, char* str2) {
/* Concat strings */
    //new strlen with null terminator
    int total_length = strlen(str1) + strlen(str2) + 1;
    char* specifier = malloc(total_length);
    if (specifier == NULL) return NULL;
    //concat strings
    snprintf(specifier, total_length, "%s%s", str1, str2);
    return specifier;
}