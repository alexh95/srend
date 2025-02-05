#include "platform.h"

typedef struct
{
    union
    {
        struct
        {
            s32 X;
            s32 Y;
        };
        s32 E[2];
    };
} v2s32;

typedef struct
{
    union
    {
        struct
        {
            f64 X;
            f64 Y;
            f64 Z;
            f64 W;
        };
        struct
        {
            f64 R;
            f64 G;
            f64 B;
            f64 A;
        };
        f64 E[4];
    };
} v4f64;

inline v2s32 V2s32(s32 X, s32 Y)
{
    v2s32 Result = { X, Y };
    return Result;
}

inline v2s32 V2s32Add(v2s32 A, v2s32 B)
{
    v2s32 Result = V2s32(A.X + B.X, A.Y + B.Y);
    return Result;
}

inline v2s32 V2s32Div(v2s32 V, s32 D)
{
    v2s32 Result = V2s32(V.X / D, V.Y / D);
    return Result;
}

inline v4f64 V4f64(f64 X, f64 Y, f64 Z, f64 W)
{
    v4f64 Result = { X, Y, Z, W };
    return Result;
}
