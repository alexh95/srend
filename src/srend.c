#include <math.h>

#include "string.c"
#include "vector.c"
#include "matrix.c"
#include "srend.h"

#define PI 3.141592f
#define PI64 3.14159265358979323846

static u32 GlobalFrameCounter = 0;

static v4f32 GlobalRed = {{ 1.0f, 0.0f, 0.0f, 1.0f }};
static v4f32 GlobalGreen = {{ 0.0f, 1.0f, 0.0f, 1.0f }};
static v4f32 GlobalBlue = {{ 0.0f, 0.0f, 1.0f, 1.0f }};
static v4f32 GlobalWhite = {{ 1.0f, 1.0f, 1.0f, 1.0f }};

static v4f32 GlobalOrange = {{ 0.89f, 0.45f, 0.05f, 1.0f }};

#define CLIP_OFFSET 0

static inline u32 ToPixelColor(v4f32 Color)
{
    u32 Result = ((u8)(Color.A * 255.0f) << 24) | ((u8)(Color.R * 255.0f) << 16) | ((u8)(Color.G * 255.0f) << 8) | ((u8)(Color.B * 255.0f) << 0);
    return Result;
}

static inline v4f32 ClipScreenDepth(state *State, v4f32 S)
{
    return S;
}

static void DrawScrollingGradient(state *State)
{
    u8 Red = 0;

    for (u32 Y = 0; Y < State->Frame.Height; ++Y)
    {
        u8 Blue = (Y + GlobalFrameCounter / 8) & 0xFF;
        for (u32 X = 0; X < State->Frame.Width; ++X)
        {
            u8 Green = (X + GlobalFrameCounter / 8) & 0xFF;
            State->Frame.Pixels[Y * State->Frame.Width + X] = (Red << 16) | (Green << 8) | Blue;
        }
    }

}

static void DrawPixel(state *State, s32 X, s32 Y, f32 Depth, u32 C)
{
    if (X >= CLIP_OFFSET && X < (s32)(State->Frame.Width - CLIP_OFFSET) && Y >= CLIP_OFFSET && Y < (s32)(State->Frame.Height - CLIP_OFFSET))
    {
        s32 Index = Y * State->Frame.Width + X;
        f32 ExistingDepth = State->Depth.Values[Index];
        if (Depth <= ExistingDepth)
        {
            State->Depth.Values[Index] = Depth;
            State->Frame.Pixels[Index] = C;
        }
    }
}

static void DrawPoint(state *State, v4f32 P, v4f32 Color)
{
    u32 C = ToPixelColor(Color);
    DrawPixel(State, (s32)P.X, (s32)P.Y, (f32)P.Z, C);
}

static void DrawLineH(state* State, v4f32 P0, v4f32 P1, s32 C)
{
    if (P0.X > P1.X)
    {
        SWAP(P0, P1);
    }

    s32 DX = (s32)(P1.X - P0.X);
    s32 DY = (s32)(P1.Y - P0.Y);

    s32 Direction = (DY < 0) ? -1 : 1;
    DY *= Direction;

    if (DX != 0)
    {
        s32 Y = (s32)P0.Y;
        s32 DecisionValue = 2 * DY - DX;
        for (s32 X = (s32)P0.X; X <= (s32)P1.X; ++X)
        {
            s32 Index = Y * State->Frame.Width + X;

            f32 DistT = (P1.X - (f32)X) / (f32)DX;
            f32 Depth = (f32)(P0.Z * DistT + P1.Z * (1.0 - DistT)) - 0.0001f;
            DrawPixel(State, X, Y, Depth, C);

            if (DecisionValue >= 0)
            {
                Y += Direction;
                DecisionValue -= 2 * DX;
            }
            DecisionValue += 2 * DY;
        }
    }
}

static void DrawLineV(state* State, v4f32 P0, v4f32 P1, s32 C)
{
    if (P0.Y > P1.Y)
    {
        SWAP(P0, P1);
    }

    s32 DX = (s32)(P1.X - P0.X);
    s32 DY = (s32)(P1.Y - P0.Y);

    s32 Direction = (DX < 0) ? -1 : 1;
    DX *= Direction;

    if (DY != 0)
    {
        s32 X = (s32)P0.X;
        s32 DecisionValue = 2 * DX - DY;
        for (s32 Y = (s32)P0.Y; Y <= (s32)P1.Y; ++Y)
        {
            s32 Index = Y * State->Frame.Width + X;

            f32 DistT = (P1.Y - (f32)Y) / (f32)DY;
            f32 Depth = (f32)(P0.Z * DistT + P1.Z * (1.0 - DistT)) - 0.0001f;
            DrawPixel(State, X, Y, Depth, C);

            if (DecisionValue >= 0)
            {
                X += Direction;
                DecisionValue -= 2 * DY;
            }
            DecisionValue += 2 * DX;
        }
    }
}

static void DrawLine(state* State, v4f32 P0, v4f32 P1, v4f32 Color)
{
    s32 C = ToPixelColor(Color);
    P0 = ClipScreenDepth(State, P0);
    P1 = ClipScreenDepth(State, P1);
    if (ABS(P1.X - P0.X) > ABS(P1.Y - P0.Y))
    {
        DrawLineH(State, P0, P1, C);
    }
    else
    {
        DrawLineV(State, P0, P1, C);
    }
}

static void DrawRect(state *State, v4f32 P0, v4f32 P1, f32 Depth, v4f32 Color)
{
    v4f32 BL = V4f32(MIN(P0.X, P1.X), MIN(P0.Y, P1.Y), Depth, 0.0);
    v4f32 BR = V4f32(MAX(P0.X, P1.X), MIN(P0.Y, P1.Y), Depth, 0.0);
    v4f32 TL = V4f32(MIN(P0.X, P1.X), MAX(P0.Y, P1.Y), Depth, 0.0);
    v4f32 TR = V4f32(MAX(P0.X, P1.X), MAX(P0.Y, P1.Y), Depth, 0.0);

    DrawLine(State, BR, TR, Color);
    DrawLine(State, TR, TL, Color);
    DrawLine(State, TL, BL, Color);
    DrawLine(State, BL, BR, Color);
}

static void DrawTriangle(state *State, v4f32 P0, v4f32 P1, v4f32 P2, v4f32 Color)
{
    DrawLine(State, P0, P1, Color);
    DrawLine(State, P1, P2, Color);
    DrawLine(State, P2, P0, Color);
}

static s32 EdgeFunction(v2s32 P0, v2s32 P1, v2s32 P2)
{
    s32 Result = (P1.X - P0.X) * (P2.Y - P0.Y) - (P1.Y - P0.Y) * (P2.X - P0.X);
    return Result;
}

static f32 EdgeFunctionDepth(v4f32 P0, v4f32 P1, v4f32 P2)
{
    f32 Result = (P1.X - P0.X) * (P2.Y - P0.Y) - (P1.Y - P0.Y) * (P2.X - P0.X);
    return Result;
}

static f32 PointInsideTriangle(v4f32 P, v4f32 P0, v4f32 P1, v4f32 P2)
{
    f32 E = EdgeFunctionDepth(P0, P1, P2);
    f32 E0 = EdgeFunctionDepth(P0, P1, P);
    f32 E1 = EdgeFunctionDepth(P1, P2, P);
    f32 E2 = EdgeFunctionDepth(P2, P0, P);

    b32 Inside = (E0 >= 0.0f) && (E1 >= 0.0f) && (E2 >= 0.0f);
    if (Inside)
    {
        f32 W0 = E0 / E;
        f32 W1 = E1 / E;
        f32 W2 = E2 / E;
        f32 Result = W0 * P0.Z + W1 * P1.Z + W2 * P2.Z;
        return Result;
    }
    return -1.0;
}

static void RasterizeTriangle(state *State, v4f32 P0, v4f32 P1, v4f32 P2, v4f32 C)
{
    u32 Color = ToPixelColor(C);

    s32 MinX = (s32)MIN4(State->Frame.Width - 1.0, P0.X, P1.X, P2.X);
    s32 MaxX = (s32)MAX4(0, P0.X, P1.X, P2.X);
    s32 MinY = (s32)MIN4(State->Frame.Height - 1.0, P0.Y, P1.Y, P2.Y);
    s32 MaxY = (s32)MAX4(0, P0.Y, P1.Y, P2.Y);

    f32 E = EdgeFunctionDepth(P0, P1, P2);

    for (s32 Y = MinY; Y <= MaxY; ++Y)
    {
        for (s32 X = MinX; X <= MaxX; ++X)
        {
            v4f32 P = V4f32((f32)X, (f32)Y, 0.0f, 0.0f);

            f32 E0 = EdgeFunctionDepth(P0, P1, P);
            f32 E1 = EdgeFunctionDepth(P1, P2, P);
            f32 E2 = EdgeFunctionDepth(P2, P0, P);

            b32 Inside = (E0 >= 0.0) && (E1 >= 0.0) && (E2 >= 0.0);
            if (Inside)
            {
                f32 W0 = E0 / E;
                f32 W1 = E1 / E;
                f32 W2 = E2 / E;
                // f32 WS = W0 + W1 + W2;

                f32 Depth = (f32)(W0 * P0.Z + W1 * P1.Z + W2 * P2.Z);
                DrawPixel(State, X, Y, Depth, Color);
            }
        }
    }
}

// static void DrawMesh_(state *State, v2s32 *Points, s32 PointCount, s32 (*Triangles)[3], s32 TriangleCount)
// {
//     for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
//     {
//         s32 *Triangle = Triangles[TriangleIndex];
//         RasterizeTriangle(State, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalWhite);
//     }

//     for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
//     {
//         s32 *Triangle = Triangles[TriangleIndex];
//         DrawTriangle(State, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalRed);
//     }

//     for (s32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
//     {
//         v2s32 Point = Points[PointIndex];
//         DrawPoint(State, Point, GlobalGreen);
//     }
// }

// static v2s32 ToScreenSpace(v4f32 P, v2s32 Offset)
// {
//     v2s32 Result = V2s32(
//         (s32)(P.X * 150.0 + Offset.X),
//         (s32)(P.Z * 150.0 + Offset.Y)
//     );
//     return Result;
// }

// static v2s32 ToScreenSpaceObj(v4f32 P)
// {
//     v2s32 Result = V2s32(
//         (s32)(P.X * 300.0 + 1000.0),
//         (s32)(P.Y * 300.0 + 300.0)
//     );
//     return Result;
// }

// static void DrawMesh(state *State, object Mesh, v2s32 Offset)
// {
//     for (u32 TriangleIndex = 0; TriangleIndex < Mesh.TriangleCount; ++TriangleIndex)
//     {
//         u32 *Triangle = Mesh.Triangles + 3 * TriangleIndex;
//         RasterizeTriangle(
//             State, 
//             ToScreenSpace(Mesh.Vertices[Triangle[0]], Offset), 
//             ToScreenSpace(Mesh.Vertices[Triangle[1]], Offset), 
//             ToScreenSpace(Mesh.Vertices[Triangle[2]], Offset), 
//             Mesh.Color
//         );
//     }

//     for (u32 TriangleIndex = 0; TriangleIndex < Mesh.TriangleCount; ++TriangleIndex)
//     {
//         u32 *Triangle = Mesh.Triangles + 3 * TriangleIndex;
//         DrawTriangle(
//             State, 
//             ToScreenSpace(Mesh.Vertices[Triangle[0]], Offset), 
//             ToScreenSpace(Mesh.Vertices[Triangle[1]], Offset), 
//             ToScreenSpace(Mesh.Vertices[Triangle[2]], Offset), 
//             GlobalRed
//         );
//     }

//     for (u32 PointIndex = 0; PointIndex < Mesh.VertexCount; ++PointIndex)
//     {
//         v2s32 Point = ToScreenSpace(Mesh.Vertices[PointIndex], Offset);
//         DrawPoint(State, Point, GlobalGreen);
//     }
// }

// static void DrawObject(state *State, object Object)
// {
//     for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
//     {
//         u32 *Triangle = Object.Triangles + 3 * TriangleIndex;
//         RasterizeTriangle(
//             State, 
//             ToScreenSpaceObj(Object.Vertices[Triangle[0]]),
//             ToScreenSpaceObj(Object.Vertices[Triangle[1]]),
//             ToScreenSpaceObj(Object.Vertices[Triangle[2]]),
//             Object.Color
//         );
//     }

//     for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
//     {
//         u32 *Triangle = Object.Triangles + 3 * TriangleIndex;
//         DrawTriangle(
//             State, 
//             ToScreenSpaceObj(Object.Vertices[Triangle[0]]),
//             ToScreenSpaceObj(Object.Vertices[Triangle[1]]),
//             ToScreenSpaceObj(Object.Vertices[Triangle[2]]),
//             GlobalRed
//         );
//     }

//     for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
//     {
//         v2s32 Point = ToScreenSpaceObj(Object.Vertices[PointIndex]);
//         DrawPoint(State, Point, GlobalGreen);
//     }
// }

object Mesh1 = {0};
object Mesh2 = {0};
object Teapot = {0};
object Compass = {0};
v4f32 GlobalOffset = {0};
v4f32 GlobalRotation = {0};
v4f32 GlobalScale = {{ 1.0f, 1.0f, 1.0f, 1.0f }};

static m4f32 Model(v4f32 Translation, v4f32 Rotation, v4f32 Scale)
{
    m4f32 TranslationMatrix = M4f32(
        1.0f, 0.0f, 0.0f, Translation.X,
        0.0f, 1.0f, 0.0f, Translation.Y,
        0.0f, 0.0f, 1.0f, Translation.Z,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    f32 SinX = sinf(Rotation.X);
    f32 CosX = cosf(Rotation.X);
    f32 SinY = sinf(Rotation.Y);
    f32 CosY = cosf(Rotation.Y);
    f32 SinZ = sinf(Rotation.Z);
    f32 CosZ = cosf(Rotation.Z);
    m4f32 RotationXMatrix = M4f32(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, CosX, -SinX, 0.0f,
        0.0f, SinX, CosX, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    m4f32 RotationYMatrix = M4f32(
        CosY, 0.0f, SinY, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -SinY, 0.0f, CosY, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    m4f32 RotationZMatrix = M4f32(
        CosZ, -SinZ, 0.0f, 0.0f,
        SinZ, CosZ, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    m4f32 RotationMatrix = M4f32Mul(RotationZMatrix, M4f32Mul(RotationYMatrix, RotationXMatrix));

    m4f32 ScaleMatrix = M4f32(
        Scale.X, 0.0f, 0.0f, 0.0f,
        0.0f, Scale.Y, 0.0f, 0.0f,
        0.0f, 0.0f, Scale.Z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    m4f32 Result = M4f32Mul(TranslationMatrix, M4f32Mul(RotationMatrix, ScaleMatrix));
    return Result;
}

static m4f32 ViewMatrix(v4f32 CameraPosition, v4f32 CameraTargetPosition, v4f32 Up)
{
    v4f32 Forward = V4f32Normalize(V4f32Sub(CameraTargetPosition, CameraPosition));

    v4f32 U = Forward;
    v4f32 V = V4f32Cross(Forward, Up);
    v4f32 W = Up;

    m4f32 Result = M4f32(
        U.X, U.Y, U.Z, -V4f32Dot(CameraPosition, U),
        V.X, V.Y, V.Z, -V4f32Dot(CameraPosition, V),
        W.X, W.Y, W.Z, -V4f32Dot(CameraPosition, W),
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return Result;
}

static m4f32 ProjectionOrtho(f32 X0, f32 X1, f32 Y0, f32 Y1, f32 Z0, f32 Z1)
{
    f32 XS = X1 + X0;
    f32 YS = Y1 + Y0;
    f32 ZS = Z1 + Z0;
    f32 XD = X1 - X0;
    f32 YD = Y1 - Y0;
    f32 ZD = Z1 - Z0;

    m4f32 Result = M4f32(
        2.0f / XD, 0.0f, 0.0f, XS / XD,
        0.0f, 2.0f / YD, 0.0f, YS / YD,
        0.0f, 0.0f, 2.0f / ZD, ZS / ZD,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return Result;
}

// NOTE(alex): maps the objects in the frustrum to [-1, 1] on Y and Z and X to [0, 1]
static m4f32 ProjectionPerspective(f32 X0, f32 X1, f32 AspectRatio, f32 FieldOfView)
{
    f32 FieldOfViewModifier = 1.0f / tanf(0.5f * FieldOfView);
    m4f32 Result = M4f32(
        1.0f / (X1 - X0), 0.0f, 0.0f, -X0 / (X1 - X0),
        0.0f, AspectRatio * FieldOfViewModifier, 0.0f, 0.0f,
        0.0f, 0.0f, FieldOfViewModifier, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f
    );
    return Result;
}

static v4f32 NormalToScreenSpace(state* State, v4f32 N)
{
    if (N.W != 0.0f)
    {
        f32 W = (N.W);
        N.X /= W;
        N.Y /= W;
        N.Z /= W;
    }

    v4f32 Result;
    Result.X = CLIP_OFFSET + 0.5f * (1.0f - N.Y) * (f32)(State->Frame.Width - 2 * CLIP_OFFSET);
    Result.Y = CLIP_OFFSET + 0.5f * (1.0f + N.Z) * (f32)(State->Frame.Height - 2 * CLIP_OFFSET);
    Result.Z = N.X;
    Result.W = N.W;

    return Result;
}

static inline b32 NormalInFront(v4f32 N)
{
    b32 Result = (N.Z >= 0.0f) && (N.Z <= 1.0f) && (N.W > 0.0f);
    return Result;
}

// NOTE(alex): Side 0 is Right, 1 is Top, 2 is Left and 3 is Bottom
static b32 IsInsideClipSide(state *State, v4f32 V, u32 Side)
{
    b32 Result = FALSE;
    switch (Side)
    {
        case 0:
        {
            Result = (s32)V.X <= ((s32)State->Frame.Width - 1 - CLIP_OFFSET);
        } break;
        case 1:
        {
            Result = (s32)V.Y <= ((s32)State->Frame.Height - 1 - CLIP_OFFSET);
        } break;
        case 2:
        {
            Result = (s32)V.X >= CLIP_OFFSET;
        } break;
        case 3:
        {
            Result = (s32)V.Y >= CLIP_OFFSET;
        } break;
        default: InvalidCodePath;
    }
    return Result;
}

static v4f32 ClipVertexSide(state *State, v4f32 V0, v4f32 V1, u32 Side)
{
    f32 SideL = (f32)CLIP_OFFSET;
    f32 SideB = (f32)CLIP_OFFSET;
    f32 SideR = (f32)(State->Frame.Width - 1 - CLIP_OFFSET);
    f32 SideT = (f32)(State->Frame.Height - 1 - CLIP_OFFSET);

    v4f32 Result = {0};
    switch (Side)
    {
        case 0:
        {
            f32 X = SideR;
            f32 T = (X - V0.X) / (V1.X - V0.X);
            f32 Y = V0.Y + T * (V1.Y - V0.Y);
            f32 Z = V0.Z + T * (V1.Z - V0.Z);
            f32 W = V0.W + T * (V1.W - V0.W);
            Result = V4f32(X, Y, Z, W);
        } break;
        case 1:
        {
            f32 Y = SideT;
            f32 T = (Y - V0.Y) / (V1.Y - V0.Y);
            f32 X = V0.X + T * (V1.X - V0.X);
            f32 Z = V0.Z + T * (V1.Z - V0.Z);
            f32 W = V0.W + T * (V1.W - V0.W);
            Result = V4f32(X, Y, Z, W);
        } break;
        case 2:
        {
            f32 X = SideL;
            f32 T = (X - V0.X) / (V1.X - V0.X);
            f32 Y = V0.Y + T * (V1.Y - V0.Y);
            f32 Z = V0.Z + T * (V1.Z - V0.Z);
            f32 W = V0.W + T * (V1.W - V0.W);
            Result = V4f32(X, Y, Z, W);
        } break;
        case 3:
        {
            f32 Y = SideB;
            f32 T = (Y - V0.Y) / (V1.Y - V0.Y);
            f32 X = V0.X + T * (V1.X - V0.X);
            f32 Z = V0.Z + T * (V1.Z - V0.Z);
            f32 W = V0.W + T * (V1.W - V0.W);
            Result = V4f32(X, Y, Z, W);
        } break;
        default: InvalidCodePath;
    }
    return Result;
}

#define MAX_CLIPPED_VERTEX_COUNT 7

static void DrawObject3D(state *State, object Object)
{
    // for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    // {
    //     u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

    //     v4f32 W0 = Object.Vertices[Triangle[0]];
    //     v4f32 W1 = Object.Vertices[Triangle[1]];
    //     v4f32 W2 = Object.Vertices[Triangle[2]];

    //     v4f32 N0 = M4f32MulV(State->MVP, W0);
    //     v4f32 N1 = M4f32MulV(State->MVP, W1);
    //     v4f32 N2 = M4f32MulV(State->MVP, W2);

    //     v4f32 S0 = NormalToScreenSpace(State, N0);
    //     v4f32 S1 = NormalToScreenSpace(State, N1);
    //     v4f32 S2 = NormalToScreenSpace(State, N2);

    //     if (NormalInFront(S0) && NormalInFront(S1) && NormalInFront(S2))
    //     {
    //         f32 DistAvg =  (S0.Z + S1.Z + S2.Z) / 3.0;
    //         v4f32 Color = V4f32(0.0, 0.0, 1.0 - DistAvg, 0.0);
    //         RasterizeTriangle(State, S0, S1, S2, Color);
    //     }
    // }

    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

        v4f32 W0 = Object.Vertices[Triangle[0]];
        v4f32 W1 = Object.Vertices[Triangle[1]];
        v4f32 W2 = Object.Vertices[Triangle[2]];

        v4f32 N0 = M4f32MulV(State->MVP, W0);
        v4f32 N1 = M4f32MulV(State->MVP, W1);
        v4f32 N2 = M4f32MulV(State->MVP, W2);

        v4f32 S0 = NormalToScreenSpace(State, N0);
        v4f32 S1 = NormalToScreenSpace(State, N1);
        v4f32 S2 = NormalToScreenSpace(State, N2);

        if (NormalInFront(S0) && NormalInFront(S1) && NormalInFront(S2))
        {
            u32 ClippedVertexCount = 3;
            v4f32 ClippedVertices[MAX_CLIPPED_VERTEX_COUNT] = {S0, S1, S2};
            v4f32 NewClippedVertices[MAX_CLIPPED_VERTEX_COUNT] = {0};
            
            for (u32 SideIndex = 0; SideIndex < 4; ++SideIndex)
            {
                u32 NewClippedVertexCount = 0;
                for (u32 VertexIndex = 0; VertexIndex < ClippedVertexCount; ++VertexIndex)
                {
                    v4f32 Vertex = ClippedVertices[VertexIndex];
                    v4f32 NextVertex = ClippedVertices[(VertexIndex + 1) % ClippedVertexCount];

                    b32 VertexIsInside = IsInsideClipSide(State, Vertex, SideIndex);
                    b32 NextVertexIsInside = IsInsideClipSide(State, NextVertex, SideIndex);

                    if (VertexIsInside)
                    {
                        if (NextVertexIsInside)
                        {
                            NewClippedVertices[NewClippedVertexCount++] = NextVertex;
                        }
                        else
                        {
                            v4f32 ClippedVertex = ClipVertexSide(State, Vertex, NextVertex, SideIndex);
                            NewClippedVertices[NewClippedVertexCount++] = ClippedVertex;
                        }
                    }
                    else
                    {
                        if (NextVertexIsInside)
                        {
                            v4f32 ClippedVertex = ClipVertexSide(State, Vertex, NextVertex, SideIndex);
                            NewClippedVertices[NewClippedVertexCount++] = ClippedVertex;
                            NewClippedVertices[NewClippedVertexCount++] = NextVertex;
                        }
                    }
                }
                ClippedVertexCount = NewClippedVertexCount;
                for (u32 Index = 0; Index < NewClippedVertexCount; ++Index)
                {
                    ClippedVertices[Index] = NewClippedVertices[Index];
                }
            }

            // u32 NewTriangleCount = ClippedVertexCount - 2;
            v4f32 C0 = ClippedVertices[0];
            v4f32 C1 = ClippedVertices[1];

            for (u32 ClippedVertexIndex = 2; ClippedVertexIndex < ClippedVertexCount; ++ClippedVertexIndex)
            {
                v4f32 C2 = ClippedVertices[ClippedVertexIndex];
                RasterizeTriangle(State, C0, C1, C2, GlobalWhite);
                DrawTriangle(State, C0, C1, C2, GlobalOrange);
                C1 = C2;
            }
        }
    }

    for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
    {
        v4f32 N = NormalToScreenSpace(State, M4f32MulV(State->MVP, Object.Vertices[PointIndex]));
        if (NormalInFront(N))
        {
            DrawPoint(State, N, GlobalGreen);
        }
    }
}

// NOTE(alex): this parses very rudimentary OBJ files
static object ParseObject(state *State, c8 *FilePath)
{
    memory_arena *Arena = &State->Arena;
    buffer ObjFile = State->Platform.OpenAndReadFile(FilePath);

    string_list Lines = StringSplitLines(Arena, ObjFile);

    u32 FirstVertexIndex = 0;
    u32 VertexCount = 0;
    u32 FirstFaceIndex = 0;
    u32 FaceCount = 0;
    u32 TriangleCount = 0;
    for (u32 LineIndex = 0; LineIndex < Lines.Count; ++LineIndex)
    {
        string Line = Lines.Strings[LineIndex];
        if (Line.Size > 0)
        {
            c8 FirstCharacter = Line.Bytes.Char[0];
            if (FirstCharacter == 'v')
            {
                if (VertexCount == 0)
                {
                    FirstVertexIndex = LineIndex;
                }
                ++VertexCount;
            }
            else if (FirstCharacter == 'f')
            {
                if (FaceCount == 0)
                {
                    FirstFaceIndex = LineIndex;
                }
                ++FaceCount;

                umm LineVertexCount = StringCountOccurrenceFrom(Line, 2, ' ') + 1;
                Assert(LineVertexCount >= 3);
                TriangleCount += (u32)LineVertexCount - 2;
            }
        }
    }

    v4f32 *Vertices = ArenaPushArray(Arena, v4f32, VertexCount);
    for (u32 VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
    {
        u32 LineIndex = VertexIndex + FirstVertexIndex;
        string Line = Lines.Strings[LineIndex];
        if (Line.Size > 2)
        {
            string_list NumberParts = StringSplitFrom(Arena, Line, 2, ' ');
            f32 X = StringParseF32(NumberParts.Strings[0]);
            f32 Y = StringParseF32(NumberParts.Strings[1]);
            f32 Z = StringParseF32(NumberParts.Strings[2]);
            v4f32 Vertex = V4f32(X, Y, Z, 1.0);
            Vertices[VertexIndex] = Vertex;
        }
    }

    u32 *Triangles = ArenaPushArray(Arena, u32, 3 * TriangleCount);
    u32 TriangleVertexIndex = 0;
    for (u32 FaceIndex = 0; FaceIndex < FaceCount; ++FaceIndex)
    {
        u32 LineIndex = FaceIndex + FirstFaceIndex;
        string Line = Lines.Strings[LineIndex];
        string_list NumberParts = StringSplitFrom(Arena, Line, 2, ' ');

        // NOTE(alex): Naive triangulation assuming the face vertices are clockwise
        s32 VertexIndex0 = StringParseS32(NumberParts.Strings[0]) - 1;
        s32 VertexIndex1 = StringParseS32(NumberParts.Strings[1]) - 1;
        for (u32 FaceVertexIndex = 2; FaceVertexIndex < NumberParts.Count; ++FaceVertexIndex)
        {
            s32 VertexIndex2 = StringParseS32(NumberParts.Strings[FaceVertexIndex]) - 1;

            Triangles[TriangleVertexIndex++] = VertexIndex2;
            Triangles[TriangleVertexIndex++] = VertexIndex1;
            Triangles[TriangleVertexIndex++] = VertexIndex0;

            VertexIndex1 = VertexIndex2;
        }
    }

    object Result = {0};
    Result.Vertices = Vertices;
    Result.VertexCount = VertexCount;
    Result.Triangles = Triangles;
    Result.TriangleCount = TriangleCount;
    Result.Color = GlobalWhite;
    return Result;
}

static void DrawWorldGrid(state *State)
{
    for (s32 X = 0; X <= 0; ++X)
    {
        v4f32 G0 = V4f32((f32)X, -10.0f, 0.0f, 1.0f);
        v4f32 G1 = V4f32((f32)X, +10.0f, 0.0f, 1.0f);

        v4f32 N0 = M4f32MulV(State->MVP, G0);
        v4f32 N1 = M4f32MulV(State->MVP, G1);

        v4f32 S0 = NormalToScreenSpace(State, N0);
        v4f32 S1 = NormalToScreenSpace(State, N1);

        if (NormalInFront(S0) && NormalInFront(S1))
        {
            DrawLine(State, S0, S1, GlobalRed);
        }
    }

    for (s32 Y = 0; Y <= 0; ++Y)
    {
        v4f32 G0 = V4f32(-10.0f, (f32)Y, 0.0f, 1.0f);
        v4f32 G1 = V4f32(+10.0f, (f32)Y, 0.0f, 1.0f);

        v4f32 N0 = M4f32MulV(State->MVP, G0);
        v4f32 N1 = M4f32MulV(State->MVP, G1);

        v4f32 S0 = NormalToScreenSpace(State, N0);
        v4f32 S1 = NormalToScreenSpace(State, N1);

        if (NormalInFront(S0) && NormalInFront(S1))
        {
            DrawLine(State, S0, S1, GlobalGreen);
        }
    }

    {
        v4f32 G0 = V4f32(0.0f, 0.0f, -10.0f, 1.0f);
        v4f32 G1 = V4f32(0.0f, 0.0f, +10.0f, 1.0f);

        v4f32 N0 = M4f32MulV(State->MVP, G0);
        v4f32 N1 = M4f32MulV(State->MVP, G1);

        v4f32 S0 = NormalToScreenSpace(State, N0);
        v4f32 S1 = NormalToScreenSpace(State, N1);

        if (NormalInFront(S0) && NormalInFront(S1))
        {
            DrawLine(State, S0, S1, GlobalBlue);
        }
    }
}

v4f32 CameraPosition = {{ -4.0f, 0.0f, 2.0f, 1.0f }};
v4f32 CameraRotation = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

static void DrawUI(state *State)
{
    DrawRect(State, V4f32(0.0f, 0.0f, 0.0f, 1.0f), V4f32(64.0f, 64.0f, 0.0f, 1.0f), -1.0f, GlobalRed);

    s32 CX = CLAMP(-(s32)CameraPosition.Y * 2, -32, 32) + 32;
    s32 CY = CLAMP((s32)CameraPosition.X * 2, -32, 32) + 32;
    DrawPixel(State, CX, CY, -2.0f, ToPixelColor(GlobalWhite));
}

extern RENDERER_INITIALIZE(RendererInitialize)
{
    memory_arena *Arena = &State->Arena;
    Mesh1 = ParseObject(State, "..\\res\\plane.obj");
    Mesh2 = ParseObject(State, "..\\res\\plane_2.obj");
    Teapot = ParseObject(State, "..\\res\\teapot_2.obj");

    int a = 0;
}

extern RENDERER_UPDATE_AND_DRAW(RendererUpdateAndDraw)
{
    for (u32 Y = 0; Y < State->Depth.Height; ++Y)
    {
        for (u32 X = 0; X < State->Depth.Width; ++X)
        {
            u32 Index = Y * State->Depth.Width + X;
            State->Depth.Values[Index] = 1.0f;
        }
    }
    DrawScrollingGradient(State);

    // v4f32 Color = V4f32(1.0, (GlobalFrameCounter & 0x0FFF) / (f32)0x0FFF, 0.0, 1.0);
    ++GlobalFrameCounter;

    // DrawPoint(State, V2s32(30, 50), Color);
    // DrawLine(State, V2s32(40, 20), V2s32(120, 80), Color);

    // s32 ScanSize = 101;
    // v2s32 ScanP0 = V2s32(20, 180);
    // v2s32 ScanP1 = V2s32(ScanP0.X + ScanSize, ScanP0.Y + ScanSize);
    // v2s32 ScanCenter = V2s32Div(V2s32Add(ScanP0, ScanP1), 2);
    
    // DrawRect(State, V2s32(20, 180), V2s32(121, 281), Color);
    // s32 ScanBorderIndex = (GlobalFrameCounter / 50) % (4 * ScanSize);
    // v2s32 ScanBorderPoint = {{ 0, 0 }};
    // if (ScanBorderIndex < ScanSize)
    // {
    //     ScanBorderPoint = V2s32(ScanP0.X + ScanBorderIndex, ScanP0.Y);
    // }
    // else if (ScanBorderIndex < 2 * ScanSize)
    // {
    //     ScanBorderPoint = V2s32(ScanP1.X, ScanP0.Y + (ScanBorderIndex - ScanSize));
    // }
    // else if (ScanBorderIndex < 3 * ScanSize)
    // {
    //     ScanBorderPoint = V2s32(ScanP1.X - (ScanBorderIndex - 2 * ScanSize), ScanP1.Y);
    // }
    // else if (ScanBorderIndex < 4 * ScanSize)
    // {
    //     ScanBorderPoint = V2s32(ScanP0.X, ScanP1.Y - (ScanBorderIndex - 3 * ScanSize));
    // }
    // DrawLine(State, ScanCenter, ScanBorderPoint, Color);

    // v2s32 Trig0[] = { V2s32(300, 15), V2s32(380, 30), V2s32(350, 80) };
    // v2s32 Trig1[] = { V2s32(350, 100), V2s32(270, 200), V2s32(200, 100) };

    // s32 T0 = EdgeFunction(Trig0[2], Trig0[1], Trig0[0]);
    // s32 T1 = EdgeFunction(Trig1[0], Trig1[1], Trig1[2]);

    // RasterizeTriangle(State, Trig1[0], Trig1[1], Trig1[2], GlobalWhite);

    // DrawTriangle(State, Trig0[0], Trig0[1], Trig0[2], Color);
    // DrawTriangle(State, Trig1[0], Trig1[1], Trig1[2], Color);

    // v2s32 Points[] =
    // { 
    //     V2s32(210, 10), V2s32(240, 8), V2s32(270, 12), V2s32(310, 10), V2s32(340, 5), V2s32(370, 20),
    //     V2s32(200, 50), V2s32(260, 48), V2s32(300, 52), V2s32(315, 50), V2s32(343, 42), V2s32(370, 49),
    //     V2s32(210, 90), V2s32(240, 88), V2s32(270, 92), V2s32(310, 90), V2s32(340, 92), V2s32(370, 89),
    // };
    // s32 Triangles[][3] = {
    //     {0, 7, 6}, {0, 1, 7}, {1, 8, 7}, {1, 2, 8}, {2, 9, 8}, {2, 3, 9}, {3, 10, 9}, {3, 4, 10}, {4, 11, 10}, {4, 5, 11},
    //     {6, 13, 12}, {6, 7, 13}, {7, 14, 13}, {7, 8, 14}, {8, 15, 14}, {8, 9, 15}, {9, 16, 15}, {9, 10, 16}, {10, 17, 16}, {10, 11, 17},
    // };

    // DrawMesh_(State, Points, ArrayCount(Points), Triangles, ArrayCount(Triangles));

    // DrawRect(State, V2s32(0, 0), V2s32(State->Frame.Width - 1, State->Frame.Height - 1), Color);

    input *Input = &State->Input;
    v4f32 TDelta = V4f32(0.0f, 0.0f, 0.0f, 0.0f);
    v4f32 RDelta = V4f32(0.0f, 0.0f, 0.0f, 0.0f);
    if (Input->Forward)
    {
        TDelta.X += 1.0f / 64.0f;
    }
    if (Input->Backward)
    {
        TDelta.X -= 1.0f / 64.0f;
    }
    if (Input->Left)
    {
        TDelta.Y += 1.0f / 64.0f;
    }
    if (Input->Right)
    {
        TDelta.Y -= 1.0f / 64.0f;
    }
    if (Input->Up)
    {
        TDelta.Z += 1.0f / 64.0f;
    }
    if (Input->Down)
    {
        TDelta.Z -= 1.0f / 64.0f;
    }
    if (Input->RollLeft)
    {
        RDelta.X -= 1.0f / 64.0f;
    }
    if (Input->RollRight)
    {
        RDelta.X += 1.0f / 64.0f;
    }
    if (Input->PitchUp)
    {
        RDelta.Y -= 1.0f / 64.0f;
    }
    if (Input->PitchDown)
    {
        RDelta.Y += 1.0f / 64.0f;
    }
    if (Input->YawLeft)
    {
        RDelta.Z += 1.0f / 64.0f;
    }
    if (Input->YawRight)
    {
        RDelta.Z -= 1.0f / 64.0f;
    }
    CameraRotation = V4f32Add(CameraRotation, RDelta);
    CameraPosition = V4f32Add(CameraPosition, TDelta);

    // DrawMesh(State, Mesh1, V2s32(600, 200));
    // DrawMesh(State, Mesh2, V2s32(1400, 200));
    // DrawObject(State, Teapot);

    v4f32 CompassVertices[] = 
    {
        V4f32(0.0f, 0.0f, -0.5f, 1.0f), V4f32(4.0f, 0.0f, -0.5f, 1.0f), V4f32(0.0f, 2.0f, -0.5f, 1.0f),
        V4f32(0.5f, 0.5f, 0.5f, 1.0f), V4f32(2.0f, 0.5f, 0.5f, 1.0f), V4f32(0.5f, 1.0f, 0.5f, 1.0f),
    };

    u32 CompassTriangles[] =
    {
        // Bottom
        0, 1, 2,
        // Top
        3, 4, 5,
        // Back
        0, 3, 2,
        2, 3, 4,
        // Right
        0, 1, 4,
        0, 4, 3,
        // Left
        4, 1, 2,
        2, 5, 4,
    };

    Compass.Vertices = CompassVertices;
    Compass.VertexCount = ArrayCount(CompassVertices);
    Compass.Triangles = CompassTriangles;
    Compass.TriangleCount = ArrayCount(CompassTriangles) / 3;

    v4f32 SquareVertices[] =
    {
        V4f32(2.0f, -2.0f, -2.0f, 1.0f), V4f32(2.0f, -2.0f, 2.0f, 1.0f),
        V4f32(2.0f, 2.0f, 2.0f, 1.0f), V4f32(2.0f, 2.0f, -2.0f, 1.0f),
    };

    u32 SquareTriangles[] =
    {
        0, 1, 2,
        0, 2, 3,
    };

    object Square = {0};
    Square.Vertices = SquareVertices;
    Square.VertexCount = ArrayCount(SquareVertices);
    Square.Triangles = SquareTriangles;
    Square.TriangleCount = ArrayCount(SquareTriangles) / 3;

    v4f32 TriangleVertices[] = { V4f32(6.0f, -2.5f, -2.5f, 1.0f), V4f32(6.0f, +2.5f, -2.5f, 1.0f), V4f32(6.0f, +0.0f, +2.5f, 1.0f) };
    u32 TriangleTriangles[] = { 0, 1, 2 };

    object Triangle = {0};
    Triangle.Vertices = TriangleVertices;
    Triangle.VertexCount = ArrayCount(TriangleVertices);
    Triangle.Triangles = TriangleTriangles;
    Triangle.TriangleCount = ArrayCount(TriangleTriangles) / 3;

    // CameraPosition.X = 4.0f;
    m4f32 CameraRotationMatrix = Model(V4f32(0.0f, 0.0f, 0.0f, 0.0f), CameraRotation, V4f32(1.0f, 1.0f, 1.0f, 0.0f));
    v4f32 CameraForward = M4f32MulV(CameraRotationMatrix, V4f32(1.0f, 0.0f, 0.0f, 0.0f));
    v4f32 CameraTarget = V4f32Add(CameraPosition, CameraForward);
    v4f32 CameraUp = M4f32MulV(CameraRotationMatrix, V4f32(0.0f, 0.0f, 1.0f, 0.0f));

    m4f32 M = Model(GlobalOffset, GlobalRotation, GlobalScale);
    m4f32 V = ViewMatrix(CameraPosition, CameraTarget, CameraUp);
    // m4f32 P = ProjectionOrtho(0.1, 10.0f, -5.0f, 5.0f, -5.0f, 5.0f);
    m4f32 P = ProjectionPerspective(0.125f, 4.0f, (f32)State->Frame.Height / (f32)State->Frame.Width, 0.5f * PI);
    m4f32 MVP = M4f32Mul(P, M4f32Mul(V, M));

    State->MVP = MVP;

    DrawObject3D(State, Triangle);
    // DrawObject3D(State, Square);
    // DrawObject3D(State, Compass);
    // DrawObject3D(State, Teapot);

    DrawWorldGrid(State);

    DrawUI(State);
}
