#ifndef MULTIPLICATION_H
#define MULTIPLICATION_H

#include "ellpack_utility.h"

/**
 * a is eine Darstellung einer Matrix M x N entlang N kompriemiert
 * a is eine Darstellung einer Matrix N x P entlang P kompriemiert
 * result wird ein Pointer zur Darstellung der Produktmatrix M x P entlang P kompriemiert
 */
void matr_mult_ellpack(const void* a, const void* b, void* result);
#endif