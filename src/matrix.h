#ifndef MATRIX_H
#define MATRIX_H

#include "platform.h"

typedef union 
{
    f32 E[16];
    f32 C[4][4];
    struct
    {
        f32 A00, A01, A02, A03;
        f32 A10, A11, A12, A13;
        f32 A20, A21, A22, A23;
        f32 A30, A31, A32, A33;
    };
    struct
    {
        f32 R0[4];
        f32 R1[4];
        f32 R2[4];
        f32 R3[4];
    };
} m4f32;

typedef m4f32 m4;

#endif
