#include "platform.h"
#include "vector.h"

// ################
// ### VECTOR 2 ###
// ################

inline v2s32 V2s32(s32 X, s32 Y)
{
    v2s32 Result = {{ X, Y }};
    return Result;
}

inline v2s32 V2s32Add(v2s32 A, v2s32 B)
{
    v2s32 Result = V2s32(A.X + B.X, A.Y + B.Y);
    return Result;
}

inline v2s32 V2s32Sub(v2s32 A, v2s32 B)
{
    v2s32 Result = V2s32(A.X - B.X, A.Y - B.Y);
    return Result;
}

inline v2s32 V2s32Mul(v2s32 V, s32 S)
{
    v2s32 Result = V2s32(V.X * S, V.Y * S);
    return Result;
}

inline v2s32 V2s32Div(v2s32 V, s32 S)
{
    v2s32 Result = V2s32(V.X / S, V.Y / S);
    return Result;
}

// ################
// ### VECTOR 4 ###
// ################

inline v4f64 V4f64(f64 X, f64 Y, f64 Z, f64 W)
{
    v4f64 Result = {{ X, Y, Z, W }};
    return Result;
}

inline v4f64 V4f64Add(v4f64 A, v4f64 B)
{
    v4f64 Result = V4f64(A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W);
    return Result;
}

inline v4f64 V4f64Sub(v4f64 A, v4f64 B)
{
    v4f64 Result = V4f64(A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W);
    return Result;
}

inline v4f64 V4f64Mul(v4f64 V, f64 S)
{
    v4f64 Result = V4f64(V.X * S, V.Y * S, V.Z * S, V.W * S);
    return Result;
}

inline v4f64 V4f64Div(v4f64 V, f64 S)
{
    v4f64 Result = V4f64(V.X / S, V.Y / S, V.Z / S, V.W / S);
    return Result;
}
