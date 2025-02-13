#include "vector.h"
#include "matrix.h"

static inline m4f32 M4f32(
    f32 A00, f32 A01, f32 A02, f32 A03,
    f32 A10, f32 A11, f32 A12, f32 A13,
    f32 A20, f32 A21, f32 A22, f32 A23,
    f32 A30, f32 A31, f32 A32, f32 A33
)
{
    m4f32 Result =
    {{
        A00, A01, A02, A03,
        A10, A11, A12, A13,
        A20, A21, A22, A23,
        A30, A31, A32, A33,
    }};
    return Result;
}

static inline m4f32 M4f32I(void)
{
    m4f32 Result = M4f32(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return Result;
}

static inline m4f32 M4f32Z(void)
{
    m4f32 Result = M4f32(
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    );
    return Result;
}

static inline v4f32 M4f32MulV(m4f32 M, v4f32 V)
{
    v4f32 Result = V4f32(
        V.X * M.A00 + V.Y * M.A01 + V.Z * M.A02 + V.W * M.A03,
        V.X * M.A10 + V.Y * M.A11 + V.Z * M.A12 + V.W * M.A13,
        V.X * M.A20 + V.Y * M.A21 + V.Z * M.A22 + V.W * M.A23,
        V.X * M.A30 + V.Y * M.A31 + V.Z * M.A32 + V.W * M.A33
    );
    return Result;
}

static inline m4f32 M4f32Mul(m4f32 A, m4f32 B)
{
    m4f32 Result = M4f32(
        A.A00 * B.A00 + A.A01 * B.A10 + A.A02 * B.A20 + A.A03 * B.A30,
        A.A00 * B.A01 + A.A01 * B.A11 + A.A02 * B.A21 + A.A03 * B.A31,
        A.A00 * B.A02 + A.A01 * B.A12 + A.A02 * B.A22 + A.A03 * B.A32,
        A.A00 * B.A03 + A.A01 * B.A13 + A.A02 * B.A23 + A.A03 * B.A33,

        A.A10 * B.A00 + A.A11 * B.A10 + A.A12 * B.A20 + A.A13 * B.A30,
        A.A10 * B.A01 + A.A11 * B.A11 + A.A12 * B.A21 + A.A13 * B.A31,
        A.A10 * B.A02 + A.A11 * B.A12 + A.A12 * B.A22 + A.A13 * B.A32,
        A.A10 * B.A03 + A.A11 * B.A13 + A.A12 * B.A23 + A.A13 * B.A33,

        A.A20 * B.A00 + A.A21 * B.A10 + A.A22 * B.A20 + A.A23 * B.A30,
        A.A20 * B.A01 + A.A21 * B.A11 + A.A22 * B.A21 + A.A23 * B.A31,
        A.A20 * B.A02 + A.A21 * B.A12 + A.A22 * B.A22 + A.A23 * B.A32,
        A.A20 * B.A03 + A.A21 * B.A13 + A.A22 * B.A23 + A.A23 * B.A33,

        A.A30 * B.A00 + A.A31 * B.A10 + A.A32 * B.A20 + A.A33 * B.A30,
        A.A30 * B.A01 + A.A31 * B.A11 + A.A32 * B.A21 + A.A33 * B.A31,
        A.A30 * B.A02 + A.A31 * B.A12 + A.A32 * B.A22 + A.A33 * B.A32,
        A.A30 * B.A03 + A.A31 * B.A13 + A.A32 * B.A23 + A.A33 * B.A33
    );
    return Result;
}
