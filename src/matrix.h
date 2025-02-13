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

static inline m4f32 M4f32(
    f32 A00, f32 A01, f32 A02, f32 A03,
    f32 A10, f32 A11, f32 A12, f32 A13,
    f32 A20, f32 A21, f32 A22, f32 A23,
    f32 A30, f32 A31, f32 A32, f32 A33
);
static inline m4f32 M4f32I(void);
static inline m4f32 M4f32Z(void);
static inline v4f32 M4f32MulV(m4f32 M, v4f32 V);
static inline m4f32 M4f32Mul(m4f32 A, m4f32 B);

#define M4f32 M4
#define M4f32I M4I
#define M4f32Z M4Z
#define M4f32MulV M4MulV
#define M4f32Mul M4Mul

#endif
