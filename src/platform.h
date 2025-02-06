#ifndef PLATFORM_H
#define PLATFORM_H

typedef char               c8;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef float              f32;
typedef double             f64;

typedef s32                b32;

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
#define MAX(A, B) ((A > B) ? (A) : (B))
#define MIN(A, B) ((A > B) ? (B) : (A))
#define MAX3(A, B, C) (MAX(MAX(A, B), C))
#define MIN3(A, B, C) (MIN(MIN(A, B), C))

#define ABS(V) ((V > 0) ? (V) : -(V))
#define SWAP(A, B)\
{\
    typeof(A) Temp = A;\
    A = B;\
    B = Temp;\
};

typedef struct
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
} frame;

typedef struct
{
    frame Frame;
} state;

#define UPDATE_AND_DRAW(Name) void (Name)(state *State)
typedef UPDATE_AND_DRAW(update_and_draw);

#endif
