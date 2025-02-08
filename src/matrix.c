#include "vector.h"
#include "matrix.h"

static m4f64 M4f64(
    f64 A00, f64 A01, f64 A02, f64 A03,
    f64 A10, f64 A11, f64 A12, f64 A13,
    f64 A20, f64 A21, f64 A22, f64 A23,
    f64 A30, f64 A31, f64 A32, f64 A33
)
{
    m4f64 Result =
    {{
        A00, A01, A02, A03,
        A10, A11, A12, A13,
        A20, A21, A22, A23,
        A30, A31, A32, A33,
    }};
    return Result;
}

static inline m4f64 M4f64I(void)
{
    m4f64 Result = M4f64(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return Result;
}

static inline m4f64 M4f64Z(void)
{
    m4f64 Result = M4f64(
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    );
    return Result;
}

static inline v4f64 M4f64MulV(m4f64 M, v4f64 V)
{
    v4f64 Result = V4f64(
        V.X * M.A00 + V.Y * M.A01 + V.Z * M.A02 + V.W * M.A03,
        V.X * M.A10 + V.Y * M.A11 + V.Z * M.A12 + V.W * M.A13,
        V.X * M.A20 + V.Y * M.A21 + V.Z * M.A22 + V.W * M.A23,
        V.X * M.A30 + V.Y * M.A31 + V.Z * M.A32 + V.W * M.A33
    );
    return Result;
}

static inline m4f64 M4f64Mul(m4f64 A, m4f64 B)
{
    m4f64 Result = M4f64(
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
