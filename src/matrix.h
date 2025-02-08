#ifndef MATRIX_H
#define MATRIX_H

#include "platform.h"

typedef union 
{
    f64 E[16];
    f64 C[4][4];
    struct
    {
        f64 A00, A01, A02, A03;
        f64 A10, A11, A12, A13;
        f64 A20, A21, A22, A23;
        f64 A30, A31, A32, A33;
    };
    struct
    {
        f64 R0[4];
        f64 R1[4];
        f64 R2[4];
        f64 R3[4];
    };
} m4f64;

typedef m4f64 m4;

#endif
