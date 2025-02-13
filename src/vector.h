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
    f32 E[4];
    struct
    {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };
    struct
    {
        f32 R;
        f32 G;
        f32 B;
        f32 A;
    };
} v4f32;

typedef v2s32 v2s;
typedef v4f32 v4;

#endif
