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

static inline v2s32 V2s32(s32 X, s32 Y);
static inline v2s32 V2s32Add(v2s32 A, v2s32 B);
static inline v2s32 V2s32Sub(v2s32 A, v2s32 B);
static inline v2s32 V2s32Mul(v2s32 V, s32 S);
static inline v2s32 V2s32Div(v2s32 V, s32 S);

#define V2s32 V2s
#define V2s32Add V2sAdd
#define V2s32Sub V2sSub
#define V2s32Mul V2sMul
#define V2s32Div V2sDiv

static inline v4f32 V4f32(f32 X, f32 Y, f32 Z, f32 W);
static inline v4f32 V4f32Add(v4f32 A, v4f32 B);
static inline v4f32 V4f32Sub(v4f32 A, v4f32 B);
static inline v4f32 V4f32Mul(v4f32 V, f32 S);
static inline v4f32 V4f32Div(v4f32 V, f32 S);
static inline f32 V4f32Dot(v4f32 A, v4f32 B);
static inline v4f32 V4f32Cross(v4f32 A, v4f32 B);
static inline f32 V4f32Length(v4f32 V);
static inline v4f32 V4f32Normalize(v4f32 V);

#define V4f32 V4
#define V4f32Add V4Add
#define V4f32Sub V4Sub
#define V4f32Mul V4Mul
#define V4f32Div V4Div
#define V4f32Dot V4Dot
#define V4f32Cross V4Cross
#define V4f32Length V4Length
#define V4f32Normalize V4Normalize

#endif
