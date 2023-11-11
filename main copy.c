#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <error.h>
#include <stdbool.h>

#include "functionality/ellpack_utility.h"
#include "functionality/multiplication.h"
#include "functionality/parser.h"


int main (int argc, char** argv) {
    struct EllpackMatrix* amatrix = parse_matrix("a.mat");
    struct EllpackMatrix* bmatrix = parse_matrix("b.mat");

    struct EllpackMatrix* result = calloc(1, sizeof(*result));
    matr_mult_ellpack(amatrix, bmatrix, result);

    write_matrix(result, "out.mat");
    printf("[FREE] Freeing used memory ...\n");
    free_all((struct EllpackMatrix *[]){amatrix, bmatrix, result}, 3);
}
