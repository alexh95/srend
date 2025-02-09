#include "platform.h"
#include "vector.h"

// ################
// ### VECTOR 2 ###
// ################

static inline v2s32 V2s32(s32 X, s32 Y)
{
    v2s32 Result = {{ X, Y }};
    return Result;
}

static inline v2s32 V2s32Add(v2s32 A, v2s32 B)
{
    v2s32 Result = V2s32(A.X + B.X, A.Y + B.Y);
    return Result;
}

static inline v2s32 V2s32Sub(v2s32 A, v2s32 B)
{
    v2s32 Result = V2s32(A.X - B.X, A.Y - B.Y);
    return Result;
}

static inline v2s32 V2s32Mul(v2s32 V, s32 S)
{
    v2s32 Result = V2s32(V.X * S, V.Y * S);
    return Result;
}

static inline v2s32 V2s32Div(v2s32 V, s32 S)
{
    v2s32 Result = V2s32(V.X / S, V.Y / S);
    return Result;
}

// ################
// ### VECTOR 4 ###
// ################

static inline v4f64 V4f64(f64 X, f64 Y, f64 Z, f64 W)
{
    v4f64 Result = {{ X, Y, Z, W }};
    return Result;
}

static inline v4f64 V4f64Add(v4f64 A, v4f64 B)
{
    v4f64 Result = V4f64(A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W);
    return Result;
}

static inline v4f64 V4f64Sub(v4f64 A, v4f64 B)
{
    v4f64 Result = V4f64(A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W);
    return Result;
}

static inline v4f64 V4f64Mul(v4f64 V, f64 S)
{
    v4f64 Result = V4f64(V.X * S, V.Y * S, V.Z * S, V.W * S);
    return Result;
}

static inline v4f64 V4f64Div(v4f64 V, f64 S)
{
    v4f64 Result = V4f64(V.X / S, V.Y / S, V.Z / S, V.W / S);
    return Result;
}

static inline f64 V4f64Dot(v4f64 A, v4f64 B)
{
    f64 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Result;
}

static inline v4f64 V4f64Cross(v4f64 A, v4f64 B)
{
    v4f64 Result = V4f64(
        A.Y * B.Z - A.Z * B.Y,
        A.Z * B.X - A.X * B.Z,
        A.X * B.Y - A.Y * B.X,
        0.0
    );
    return Result;
}

static inline f64 V4f64Length(v4f64 V)
{
    f64 Result = sqrt(V4f64Dot(V, V));
    return Result;
}

static inline v4f64 V4f64Normalize(v4f64 V)
{
    f64 Length = V4f64Length(V);
    v4f64 Result = V;
    if (Length != 0.0)
    {
        Result.X /= Length;
        Result.Y /= Length;
        Result.Z /= Length;
        Result.W /= Length;
    }
    return Result;
}
