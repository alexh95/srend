#ifndef PLATFORM_H
#define PLATFORM_H

typedef size_t             umm;

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

#define FALSE 0
#define TRUE 1

#define MIN_U8                   0
#define MIN_U16                  0
#define MIN_U32                  0
#define MIN_U64                  0
#define MAX_U8                0xFF
#define MAX_U16             0xFFFF
#define MAX_U32         0xFFFFFFFF
#define MAX_U64 0xFFFFFFFFFFFFFFFF

#define MIN_S8                  -128
#define MIN_S16               -32768
#define MIN_S32          -2147483648
#define MIN_S64 -9223372036854775808
#define MAX_S8                   127
#define MAX_S16                32767
#define MAX_S32           2147483647
#define MAX_S64  9223372036854775807

#define Assert(Condition) { if(!(Condition)) { *(s32 *)0 = 0; } }
#define InvalidCodePath Assert(0)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

#define MAX(A, B) ((A > B) ? (A) : (B))
#define MIN(A, B) ((A > B) ? (B) : (A))
#define MAX3(A, B, C) (MAX(MAX(A, B), C))
#define MIN3(A, B, C) (MIN(MIN(A, B), C))
#define MAX4(A, B, C, D) (MAX(MAX(A, B), MAX(C, D)))
#define MIN4(A, B, C, D) (MIN(MIN(A, B), MIN(C, D)))

#define CLAMP(V, V0, V1) (((V) < (V0)) ? (V0) : ((V) > (V1)) ? (V1) : (V))

#define ABS(V) ((V > 0) ? (V) : -(V))
#define SWAP(A, B)\
{\
    typeof(A) Temp = A;\
    A = B;\
    B = Temp;\
};

typedef struct
{
    union
    {
        void *Data;
        union
        {
            char *Char;
            u8 *U8;
            s8 *S8;
        } Bytes;
        union
        {
            u16 *U16;
            s16 *S16;
        } Words;
        union
        {
            u32 *U32;
            s32 *S32;
        } Dwords;
        union
        {
            u64 *U64;
            s64 *S64;
        } Qwords;
    };
    umm Size;
} buffer;

typedef struct struct_memory_arena
{
    u8 *Data;
    umm Used;
    umm MaxSize;
} memory_arena;

#include "vector.h"
#include "matrix.h"

#define KiloBytes(Size) (1024LL * (Size))
#define MegaBytes(Size) (1024LL * KiloBytes(Size))
#define GigaBytes(Size) (1024LL * MegaBytes(Size))
#define TerraBytes(Size) (1024LL * GigaBytes(Size))

u8 *ArenaPushInternal(memory_arena *Arena, umm Size)
{
    if ((Arena->MaxSize - Arena->Used) < Size)
    {
        InvalidCodePath;
    }

    u8 *Result = Arena->Data + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#define ArenaPush(Arena, Type) ((Type *)ArenaPushInternal(Arena, sizeof(Type)))
#define ArenaPushArray(Arena, Type, Count) ((Type *)ArenaPushInternal(Arena, Count * sizeof(Type)))

static inline buffer ArenaPushBuffer(memory_arena *Arena, umm Size)
{
    buffer Result = {};
    Result.Data = ArenaPushArray(Arena, u8, Size);
    Result.Size = Size;
    return Result;
}

typedef struct
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
} frame;

typedef struct
{
    u32 Width;
    u32 Height;
    f32 *Values;
} depth;

#define OPEN_AND_READ_FILE(Name) buffer (Name)(c8 *FileName)
typedef OPEN_AND_READ_FILE(open_and_read_file);

typedef struct
{
    f64 deltaT;

    b32 Left;
    b32 Right;
    b32 Forward;
    b32 Backward;
    b32 Up;
    b32 Down;
    b32 RollLeft;
    b32 RollRight;
    b32 PitchUp;
    b32 PitchDown;
    b32 YawLeft;
    b32 YawRight;
} input;

typedef struct
{
    open_and_read_file *OpenAndReadFile;
} platform;

typedef struct
{
    platform Platform;
    memory_arena Arena;
    input Input;
    frame Frame;
    depth Depth;
    m4 MVP;
} state;

#define RENDERER_INITIALIZE(Name) void (Name)(state *State)
typedef RENDERER_INITIALIZE(renderer_initialize);

#define RENDERER_UPDATE_AND_DRAW(Name) void (Name)(state *State)
typedef RENDERER_UPDATE_AND_DRAW(renderer_update_and_draw);

#endif
