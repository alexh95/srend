#ifndef VECTOR_H
#define VECTOR_H

#include "vector.h"

typedef union
{
    s32 E[2];
    struct
    {
        s32 X;
        s32 Y;
    };
} v2s32;

typedef union
{
    f64 E[4];
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
} v4f64;

#endif
