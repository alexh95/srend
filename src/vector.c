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

static inline v4f32 V4f32(f32 X, f32 Y, f32 Z, f32 W)
{
    v4f32 Result = {{ X, Y, Z, W }};
    return Result;
}

static inline v4f32 V4f32Add(v4f32 A, v4f32 B)
{
    v4f32 Result = V4f32(A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W);
    return Result;
}

static inline v4f32 V4f32Sub(v4f32 A, v4f32 B)
{
    v4f32 Result = V4f32(A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W);
    return Result;
}

static inline v4f32 V4f32Mul(v4f32 V, f32 S)
{
    v4f32 Result = V4f32(V.X * S, V.Y * S, V.Z * S, V.W * S);
    return Result;
}

static inline v4f32 V4f32Div(v4f32 V, f32 S)
{
    v4f32 Result = V4f32(V.X / S, V.Y / S, V.Z / S, V.W / S);
    return Result;
}

static inline f32 V4f32Dot(v4f32 A, v4f32 B)
{
    f32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Result;
}

static inline v4f32 V4f32Cross(v4f32 A, v4f32 B)
{
    v4f32 Result = V4f32(
        A.Y * B.Z - A.Z * B.Y,
        A.Z * B.X - A.X * B.Z,
        A.X * B.Y - A.Y * B.X,
        0.0f
    );
    return Result;
}

static inline f32 V4f32Length(v4f32 V)
{
    f32 Result = sqrtf(V4f32Dot(V, V));
    return Result;
}

static inline v4f32 V4f32Normalize(v4f32 V)
{
    f32 Length = V4f32Length(V);
    v4f32 Result = V;
    if (Length != 0.0f)
    {
        Result.X /= Length;
        Result.Y /= Length;
        Result.Z /= Length;
        Result.W /= Length;
    }
    return Result;
}
