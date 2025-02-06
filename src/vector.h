#ifndef VECTOR_H
#define VECTOR_H

#include "vector.h"

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

#endif
