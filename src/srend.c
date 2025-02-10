#include <math.h>

#include "string.c"
#include "vector.c"
#include "matrix.c"
#include "srend.h"

#define PI 3.14159265358979323846

static u32 GlobalFrameCounter = 0;

static v4f64 GlobalRed = {{ 1.0, 0.0, 0.0, 1.0 }};
static v4f64 GlobalGreen = {{ 0.0, 1.0, 0.0, 1.0 }};
static v4f64 GlobalBlue = {{ 0.0, 0.0, 1.0, 1.0 }};
static v4f64 GlobalWhite = {{ 1.0, 1.0, 1.0, 1.0 }};

static inline u32 ToPixelColor(v4f64 Color)
{
    u32 Result = ((u8)(Color.A * 255.0) << 24) | ((u8)(Color.R * 255.0) << 16) | ((u8)(Color.G * 255.0) << 8) | ((u8)(Color.B * 255.0) << 0);
    return Result;
}

static inline v2s32 ClipScreen(state *State, v2s32 S)
{
    v2s32 Result = V2s32(
        CLAMP(S.X, 0, (s32)(State->Frame.Width - 1)),
        CLAMP(S.Y, 0, (s32)(State->Frame.Height - 1))
    );
    return Result;
}

static inline v4f64 ClipScreenDepth(state *State, v4f64 S)
{
    // TODO(alex): this clipping does not maintin depth properly!!
    v4f64 Result = V4f64(
        CLAMP(S.X, 0.0, (f64)(State->Frame.Width - 1)),
        CLAMP(S.Y, 0.0, (f64)(State->Frame.Height - 1)),
        S.Z,
        S.W
    );
    return Result;
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

static void DrawPixel(state *State, s32 X, s32 Y, u32 C)
{
    if (X >= 0 && X < (s32)State->Frame.Width && Y >= 0 && Y < (s32)State->Frame.Height)
    {
        s32 FrameIndex = Y * State->Frame.Width + X;
        State->Frame.Pixels[FrameIndex] = C;
    }
}

static void DrawPoint(state *State, v2s32 P, v4f64 Color)
{
    u32 C = ToPixelColor(Color);
    DrawPixel(State, P.X, P.Y, C);
}

static void DrawLineH(state* State, s32 X0, s32 Y0, s32 X1, s32 Y1, s32 C)
{
    if (X0 > X1)
    {
        SWAP(X0, X1);
        SWAP(Y0, Y1);
    }

    s32 DX = X1 - X0;
    s32 DY = Y1 - Y0;

    s32 Direction = (DY < 0) ? -1 : 1;
    DY *= Direction;

    if (DX != 0)
    {
        s32 Y = Y0;
        s32 DecisionValue = 2 * DY - DX;
        for (s32 X = X0; X <= X1; ++X)
        {
            DrawPixel(State, X, Y, C);
            if (DecisionValue >= 0)
            {
                Y += Direction;
                DecisionValue -= 2 * DX;
            }
            DecisionValue += 2 * DY;
        }
    }
}

static void DrawLineHDepth(state* State, v4f64 P0, v4f64 P1, s32 C)
{
    if (P0.X > P1.X)
    {
        SWAP(P0.X, P1.X);
        SWAP(P0.Y, P1.Y);
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

            f64 DistT = (P1.X - (f64)X) / (f64)DX;
            f32 Depth = (f32)(P0.Z * DistT + P1.Z * (1.0 - DistT)) - 0.0001f;
            f32 ExistingDepth = State->Depth.Values[Index];

            if (Depth < ExistingDepth)
            {
                State->Depth.Values[Index] = Depth;
                DrawPixel(State, X, Y, C);
            }

            if (DecisionValue >= 0)
            {
                Y += Direction;
                DecisionValue -= 2 * DX;
            }
            DecisionValue += 2 * DY;
        }
    }
}

static void DrawLineV(state* State, s32 X0, s32 Y0, s32 X1, s32 Y1, s32 C)
{
    if (Y0 > Y1)
    {
        SWAP(X0, X1);
        SWAP(Y0, Y1);
    }

    s32 DX = X1 - X0;
    s32 DY = Y1 - Y0;

    s32 Direction = (DX < 0) ? -1 : 1;
    DX *= Direction;

    if (DY != 0)
    {
        s32 X = X0;
        s32 DecisionValue = 2 * DX - DY;
        for (s32 Y = Y0; Y <= Y1; ++Y)
        {
            DrawPixel(State, X, Y, C);
            if (DecisionValue >= 0)
            {
                X += Direction;
                DecisionValue -= 2 * DY;
            }
            DecisionValue += 2 * DX;
        }
    }
}

static void DrawLineVDepth(state* State, v4f64 P0, v4f64 P1, s32 C)
{
    if (P0.X > P1.X)
    {
        SWAP(P0.X, P1.X);
        SWAP(P0.Y, P1.Y);
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

            f64 DistT = (P1.X - (f64)X) / (f64)DX;
            f32 Depth = (f32)(P0.Z * DistT + P1.Z * (1.0 - DistT)) - 0.0001f;
            f32 ExistingDepth = State->Depth.Values[Index];

            if (Depth < ExistingDepth)
            {
                State->Depth.Values[Index] = Depth;
                DrawPixel(State, X, Y, C);
            }

            if (DecisionValue >= 0)
            {
                X += Direction;
                DecisionValue -= 2 * DY;
            }
            DecisionValue += 2 * DX;
        }
    }
}

static void DrawLine(state* State, v2s32 P0, v2s32 P1, v4f64 Color)
{
    s32 C = ToPixelColor(Color);
    P0 = ClipScreen(State, P0);
    P1 = ClipScreen(State, P1);
    if (ABS(P1.X - P0.X) > ABS(P1.Y - P0.Y))
    {
        DrawLineH(State, P0.X, P0.Y, P1.X, P1.Y, C);
    }
    else
    {
        DrawLineV(State, P0.X, P0.Y, P1.X, P1.Y, C);
    }
}

static void DrawLineDepth(state* State, v4f64 P0, v4f64 P1, v4f64 Color)
{
    s32 C = ToPixelColor(Color);
    P0 = ClipScreenDepth(State, P0);
    P1 = ClipScreenDepth(State, P1);
    if (ABS(P1.X - P0.X) > ABS(P1.Y - P0.Y))
    {
        DrawLineHDepth(State, P0, P1, C);
    }
    else
    {
        DrawLineVDepth(State, P0, P1, C);
    }
}

static void DrawRect(state *State, v2s32 P0, v2s32 P1, v4f64 Color)
{
    DrawLine(State, V2s32(P0.X, P0.Y), V2s32(P1.X, P0.Y), Color);
    DrawLine(State, V2s32(P1.X, P0.Y), V2s32(P1.X, P1.Y), Color);
    DrawLine(State, V2s32(P1.X, P1.Y), V2s32(P0.X, P1.Y), Color);
    DrawLine(State, V2s32(P0.X, P1.Y), V2s32(P0.X, P0.Y), Color);
}

static void DrawTriangle(state *State, v2s32 P0, v2s32 P1, v2s32 P2, v4f64 Color)
{
    DrawLine(State, P0, P1, Color);
    DrawLine(State, P1, P2, Color);
    DrawLine(State, P2, P0, Color);
}

static void DrawTriangleDepth(state *State, v4f64 P0, v4f64 P1, v4f64 P2, v4f64 Color)
{
    DrawLineDepth(State, P0, P1, Color);
    DrawLineDepth(State, P1, P2, Color);
    DrawLineDepth(State, P2, P0, Color);
}

static s32 EdgeFunction(v2s32 P0, v2s32 P1, v2s32 P2)
{
    s32 Result = (P1.X - P0.X) * (P2.Y - P0.Y) - (P1.Y - P0.Y) * (P2.X - P0.X);
    return Result;
}

static f64 EdgeFunctionDepth(v4f64 P0, v4f64 P1, v4f64 P2)
{
    f64 Result = (P1.X - P0.X) * (P2.Y - P0.Y) - (P1.Y - P0.Y) * (P2.X - P0.X);
    return Result;
}

static b32 PointInside(v2s32 P, v2s32 P0, v2s32 P1, v2s32 P2)
{
    s32 E0 = EdgeFunction(P0, P1, P);
    s32 E1 = EdgeFunction(P1, P2, P);
    s32 E2 = EdgeFunction(P2, P0, P);
    s32 Result = (E0 >= 0) && (E1 >= 0) && (E2 >= 0);
    return Result;
}

static void RasterizeTriangle(state *State, v2s32 P0, v2s32 P1, v2s32 P2, v4f64 C)
{
    s32 MinX = MIN4((s32)(State->Frame.Width - 1), P0.X, P1.X, P2.X);
    s32 MaxX = MAX4(0, P0.X, P1.X, P2.X);
    s32 MinY = MIN4((s32)(State->Frame.Height - 1), P0.Y, P1.Y, P2.Y);
    s32 MaxY = MAX4(0, P0.Y, P1.Y, P2.Y);

    for (s32 Y = MinY; Y <= MaxY; ++Y)
    {
        for (s32 X = MinX; X <= MaxX; ++X)
        {
            v2s32 P = V2s32(X, Y);
            b32 Inside = PointInside(P, P0, P1, P2);
            if (Inside)
            {
                DrawPoint(State, P, C);
            }
        }
    }
}

static void RasterizeTriangleDepth(state *State, v4f64 P0, v4f64 P1, v4f64 P2, v4f64 C)
{
    s32 MinX = (s32)MIN4(State->Frame.Width - 1.0, P0.X, P1.X, P2.X);
    s32 MaxX = (s32)MAX4(0, P0.X, P1.X, P2.X);
    s32 MinY = (s32)MIN4(State->Frame.Height - 1.0, P0.Y, P1.Y, P2.Y);
    s32 MaxY = (s32)MAX4(0, P0.Y, P1.Y, P2.Y);

    f64 E = EdgeFunctionDepth(P0, P1, P2);

    for (s32 Y = MinY; Y <= MaxY; ++Y)
    {
        for (s32 X = MinX; X <= MaxX; ++X)
        {
            v4f64 P = V4f64(X, Y, 0.0, 0.0);

            f64 E0 = EdgeFunctionDepth(P0, P1, P);
            f64 E1 = EdgeFunctionDepth(P1, P2, P);
            f64 E2 = EdgeFunctionDepth(P2, P0, P);

            b32 Inside = (E0 >= 0.0) && (E1 >= 0.0) && (E2 >= 0.0);
            if (Inside)
            {
                f64 W0 = E0 / E;
                f64 W1 = E1 / E;
                f64 W2 = E2 / E;
                // f64 WS = W0 + W1 + W2;

                s32 Index = Y * State->Frame.Width + X;

                f32 Depth = (f32)(W0 * P0.Z + W1 * P1.Z + W2 * P2.Z);
                f32 ExistingDepth = State->Depth.Values[Index];

                if (Depth < ExistingDepth)
                {
                    State->Depth.Values[Index] = Depth;
                    DrawPoint(State, V2s32(X, Y), C);
                }
            }
        }
    }
}

static void DrawMesh_(state *State, v2s32 *Points, s32 PointCount, s32 (*Triangles)[3], s32 TriangleCount)
{
    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        RasterizeTriangle(State, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalWhite);
    }

    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        DrawTriangle(State, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalRed);
    }

    for (s32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
    {
        v2s32 Point = Points[PointIndex];
        DrawPoint(State, Point, GlobalGreen);
    }
}

static v2s32 ToScreenSpace(v4f64 P, v2s32 Offset)
{
    v2s32 Result = V2s32(
        (s32)(P.X * 150.0 + Offset.X),
        (s32)(P.Z * 150.0 + Offset.Y)
    );
    return Result;
}

static v2s32 ToScreenSpaceObj(v4f64 P)
{
    v2s32 Result = V2s32(
        (s32)(P.X * 300.0 + 1000.0),
        (s32)(P.Y * 300.0 + 300.0)
    );
    return Result;
}

static void DrawMesh(state *State, object Mesh, v2s32 Offset)
{
    for (u32 TriangleIndex = 0; TriangleIndex < Mesh.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Mesh.Triangles + 3 * TriangleIndex;
        RasterizeTriangle(
            State, 
            ToScreenSpace(Mesh.Vertices[Triangle[0]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[1]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[2]], Offset), 
            Mesh.Color
        );
    }

    for (u32 TriangleIndex = 0; TriangleIndex < Mesh.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Mesh.Triangles + 3 * TriangleIndex;
        DrawTriangle(
            State, 
            ToScreenSpace(Mesh.Vertices[Triangle[0]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[1]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[2]], Offset), 
            GlobalRed
        );
    }

    for (u32 PointIndex = 0; PointIndex < Mesh.VertexCount; ++PointIndex)
    {
        v2s32 Point = ToScreenSpace(Mesh.Vertices[PointIndex], Offset);
        DrawPoint(State, Point, GlobalGreen);
    }
}

static void DrawObject(state *State, object Object)
{
    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;
        RasterizeTriangle(
            State, 
            ToScreenSpaceObj(Object.Vertices[Triangle[0]]),
            ToScreenSpaceObj(Object.Vertices[Triangle[1]]),
            ToScreenSpaceObj(Object.Vertices[Triangle[2]]),
            Object.Color
        );
    }

    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;
        DrawTriangle(
            State, 
            ToScreenSpaceObj(Object.Vertices[Triangle[0]]),
            ToScreenSpaceObj(Object.Vertices[Triangle[1]]),
            ToScreenSpaceObj(Object.Vertices[Triangle[2]]),
            GlobalRed
        );
    }

    for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
    {
        v2s32 Point = ToScreenSpaceObj(Object.Vertices[PointIndex]);
        DrawPoint(State, Point, GlobalGreen);
    }
}

object Mesh1 = {0};
object Mesh2 = {0};
object Teapot = {0};
object Compass = {0};
v4f64 GlobalOffset = {0};
v4f64 GlobalRotation = {0};
v4f64 GlobalScale = {{ 1.0, 1.0, 1.0, 1.0 }};

static m4f64 Model(v4f64 Translation, v4f64 Rotation, v4f64 Scale)
{
    m4f64 TranslationMatrix = M4f64(
        1.0, 0.0, 0.0, Translation.X,
        0.0, 1.0, 0.0, Translation.Y,
        0.0, 0.0, 1.0, Translation.Z,
        0.0, 0.0, 0.0, 1.0
    );

    f64 SinX = sin(Rotation.X);
    f64 CosX = cos(Rotation.X);
    f64 SinY = sin(Rotation.Y);
    f64 CosY = cos(Rotation.Y);
    f64 SinZ = sin(Rotation.Z);
    f64 CosZ = cos(Rotation.Z);
    m4f64 RotationXMatrix = M4f64(
        1.0, 0.0, 0.0, 0.0,
        0.0, CosX, -SinX, 0.0,
        0.0, SinX, CosX, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m4f64 RotationYMatrix = M4f64(
        CosY, 0.0, SinY, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -SinY, 0.0, CosY, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m4f64 RotationZMatrix = M4f64(
        CosZ, -SinZ, 0.0, 0.0,
        SinZ, CosZ, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m4f64 RotationMatrix = M4f64Mul(RotationZMatrix, M4f64Mul(RotationYMatrix, RotationXMatrix));

    m4f64 ScaleMatrix = M4f64(
        Scale.X, 0.0, 0.0, 0.0,
        0.0, Scale.Y, 0.0, 0.0,
        0.0, 0.0, Scale.Z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m4f64 Result = M4f64Mul(TranslationMatrix, M4f64Mul(RotationMatrix, ScaleMatrix));
    return Result;
}

static m4f64 ViewMatrix(v4f64 CameraPosition, v4f64 CameraTargetPosition, v4f64 Up)
{
    v4f64 Forward = V4f64Normalize(V4f64Sub(CameraTargetPosition, CameraPosition));

    v4f64 U = Forward;
    v4f64 V = V4f64Cross(Forward, Up);
    v4f64 W = Up;

    m4f64 Result = M4f64(
        U.X, U.Y, U.Z, -V4f64Dot(CameraPosition, U),
        V.X, V.Y, V.Z, -V4f64Dot(CameraPosition, V),
        W.X, W.Y, W.Z, -V4f64Dot(CameraPosition, W),
        0.0, 0.0, 0.0, 1.0
    );
    return Result;
}

static m4f64 ProjectionOrtho(f64 X0, f64 X1, f64 Y0, f64 Y1, f64 Z0, f64 Z1)
{
    f64 XS = X1 + X0;
    f64 YS = Y1 + Y0;
    f64 ZS = Z1 + Z0;
    f64 XD = X1 - X0;
    f64 YD = Y1 - Y0;
    f64 ZD = Z1 - Z0;

    m4f64 Result = M4f64(
        2.0 / XD, 0.0, 0.0, XS / XD,
        0.0, 2.0 / YD, 0.0, YS / YD,
        0.0, 0.0, 2.0 / ZD, ZS / ZD,
        0.0, 0.0, 0.0, 1.0
    );
    return Result;
}

// NOTE(alex): maps the objects in the frustrum to [-1, 1] on Y and Z and X to [0, 1]
static m4f64 ProjectionPerspective(f64 X0, f64 X1, f64 AspectRatio, f64 FieldOfView)
{
    f64 FieldOfViewModifier = 1.0 / tan(0.5 * FieldOfView);
    m4f64 Result = M4f64(
        1.0 / (X1 - X0), 0.0, 0.0, -X0 / (X1 - X0),
        0.0, AspectRatio * FieldOfViewModifier, 0.0, 0.0,
        0.0, 0.0, FieldOfViewModifier, 0.0,
        1.0, 0.0, 0.0, 0.0
    );
    return Result;
}

static v4f64 NormalToScreenSpace(state* State, v4f64 N)
{
    v4f64 Result = N;

    if (N.W != 0.0)
    {
        f64 W = ABS(Result.W);
        Result.X /= W;
        Result.Y /= W;
        Result.Z /= W;
    }

    Result.Y = 0.5 * (1.0 - Result.Y) * (f64)State->Frame.Width;
    Result.Z = 0.5 * (1.0 + Result.Z) * (f64)State->Frame.Height;

    return Result;
}

static v2s32 ToScreen(v4f64 S)
{
    v2s32 Result = V2s32((s32)S.Y, (s32)S.Z);
    return Result;
}

static v4f64 ToScreenDepth(v4f64 S)
{
    v4f64 Result = V4f64(S.Y, S.Z, S.X, S.W);
    return Result;
}

static void DrawObject3D(state *State, object Object)
{
    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

        v4f64 W0 = Object.Vertices[Triangle[0]];
        v4f64 W1 = Object.Vertices[Triangle[1]];
        v4f64 W2 = Object.Vertices[Triangle[2]];

        v4f64 N0 = M4f64MulV(State->MVP, W0);
        v4f64 N1 = M4f64MulV(State->MVP, W1);
        v4f64 N2 = M4f64MulV(State->MVP, W2);

        v4f64 S0 = NormalToScreenSpace(State, N0);
        v4f64 S1 = NormalToScreenSpace(State, N1);
        v4f64 S2 = NormalToScreenSpace(State, N2);

        if (S0.X >= 0.0 && S0.X <= 1.0 && S1.X >= 0.0 && S1.X <= 1.0 && S2.X >= 0.0 && S2.X <= 1.0)
        {
            f64 DistAvg =  (S0.X + S1.X + S2.X) / 3.0;
            v4f64 Color = V4f64(0.0, 0.0, 1.0 - DistAvg, 0.0);
            RasterizeTriangleDepth(
                State, 
                ToScreenDepth(S0),
                ToScreenDepth(S1),
                ToScreenDepth(S2),
                Color
            );
        }
    }

    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

        v4f64 W0 = Object.Vertices[Triangle[0]];
        v4f64 W1 = Object.Vertices[Triangle[1]];
        v4f64 W2 = Object.Vertices[Triangle[2]];

        v4f64 N0 = M4f64MulV(State->MVP, W0);
        v4f64 N1 = M4f64MulV(State->MVP, W1);
        v4f64 N2 = M4f64MulV(State->MVP, W2);

        v4f64 S0 = NormalToScreenSpace(State, N0);
        v4f64 S1 = NormalToScreenSpace(State, N1);
        v4f64 S2 = NormalToScreenSpace(State, N2);

        if (S0.X >= 0.0 && S0.X <= 1.0 && S1.X >= 0.0 && S1.X <= 1.0 && S2.X >= 0.0 && S2.X <= 1.0)
        {
            DrawTriangleDepth(
                State, 
                ToScreenDepth(S0),
                ToScreenDepth(S1),
                ToScreenDepth(S2),
                GlobalRed
            );
        }
    }

    for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
    {
        v2s32 Point = ToScreen(NormalToScreenSpace(State, M4f64MulV(State->MVP, Object.Vertices[PointIndex])));
        DrawPoint(State, Point, GlobalGreen);
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

    v4f64 *Vertices = ArenaPushArray(Arena, v4f64, VertexCount);
    for (u32 VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
    {
        u32 LineIndex = VertexIndex + FirstVertexIndex;
        string Line = Lines.Strings[LineIndex];
        if (Line.Size > 2)
        {
            string_list NumberParts = StringSplitFrom(Arena, Line, 2, ' ');
            f64 X = StringParseF64(NumberParts.Strings[0]);
            f64 Y = StringParseF64(NumberParts.Strings[1]);
            f64 Z = StringParseF64(NumberParts.Strings[2]);
            v4f64 Vertex = V4f64(X, Y, Z, 1.0);
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

extern RENDERER_INITIALIZE(RendererInitialize)
{
    memory_arena *Arena = &State->Arena;
    Mesh1 = ParseObject(State, "..\\res\\plane.obj");
    Mesh2 = ParseObject(State, "..\\res\\plane_2.obj");
    Teapot = ParseObject(State, "..\\res\\teapot_2.obj");

    int a = 0;
}

v4f64 CameraPosition = {{ -4.0, 0.0, 1.5, 1.0 }};
v4f64 CameraRotation = {{ 0.0, 0.0, 0.0, 0.0 }};

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

    v4f64 Color = V4f64(1.0, (GlobalFrameCounter & 0x0FFF) / (f64)0x0FFF, 0.0, 1.0);
    ++GlobalFrameCounter;

    DrawScrollingGradient(State);

    DrawPoint(State, V2s32(30, 50), Color);
    DrawLine(State, V2s32(40, 20), V2s32(120, 80), Color);

    s32 ScanSize = 101;
    v2s32 ScanP0 = V2s32(20, 180);
    v2s32 ScanP1 = V2s32(ScanP0.X + ScanSize, ScanP0.Y + ScanSize);
    v2s32 ScanCenter = V2s32Div(V2s32Add(ScanP0, ScanP1), 2);
    
    DrawRect(State, V2s32(20, 180), V2s32(121, 281), Color);
    s32 ScanBorderIndex = (GlobalFrameCounter / 50) % (4 * ScanSize);
    v2s32 ScanBorderPoint = {{ 0, 0 }};
    if (ScanBorderIndex < ScanSize)
    {
        ScanBorderPoint = V2s32(ScanP0.X + ScanBorderIndex, ScanP0.Y);
    }
    else if (ScanBorderIndex < 2 * ScanSize)
    {
        ScanBorderPoint = V2s32(ScanP1.X, ScanP0.Y + (ScanBorderIndex - ScanSize));
    }
    else if (ScanBorderIndex < 3 * ScanSize)
    {
        ScanBorderPoint = V2s32(ScanP1.X - (ScanBorderIndex - 2 * ScanSize), ScanP1.Y);
    }
    else if (ScanBorderIndex < 4 * ScanSize)
    {
        ScanBorderPoint = V2s32(ScanP0.X, ScanP1.Y - (ScanBorderIndex - 3 * ScanSize));
    }
    DrawLine(State, ScanCenter, ScanBorderPoint, Color);

    v2s32 Trig0[] = { V2s32(300, 15), V2s32(380, 30), V2s32(350, 80) };
    v2s32 Trig1[] = { V2s32(350, 100), V2s32(270, 200), V2s32(200, 100) };

    s32 T0 = EdgeFunction(Trig0[2], Trig0[1], Trig0[0]);
    s32 T1 = EdgeFunction(Trig1[0], Trig1[1], Trig1[2]);

    RasterizeTriangle(State, Trig1[0], Trig1[1], Trig1[2], GlobalWhite);

    DrawTriangle(State, Trig0[0], Trig0[1], Trig0[2], Color);
    DrawTriangle(State, Trig1[0], Trig1[1], Trig1[2], Color);

    v2s32 Points[] =
    { 
        V2s32(210, 10), V2s32(240, 8), V2s32(270, 12), V2s32(310, 10), V2s32(340, 5), V2s32(370, 20),
        V2s32(200, 50), V2s32(260, 48), V2s32(300, 52), V2s32(315, 50), V2s32(343, 42), V2s32(370, 49),
        V2s32(210, 90), V2s32(240, 88), V2s32(270, 92), V2s32(310, 90), V2s32(340, 92), V2s32(370, 89),
    };
    s32 Triangles[][3] = {
        {0, 7, 6}, {0, 1, 7}, {1, 8, 7}, {1, 2, 8}, {2, 9, 8}, {2, 3, 9}, {3, 10, 9}, {3, 4, 10}, {4, 11, 10}, {4, 5, 11},
        {6, 13, 12}, {6, 7, 13}, {7, 14, 13}, {7, 8, 14}, {8, 15, 14}, {8, 9, 15}, {9, 16, 15}, {9, 10, 16}, {10, 17, 16}, {10, 11, 17},
    };

    DrawMesh_(State, Points, ArrayCount(Points), Triangles, ArrayCount(Triangles));

    DrawRect(State, V2s32(0, 0), V2s32(State->Frame.Width - 1, State->Frame.Height - 1), Color);

    input *Input = &State->Input;
    v4f64 TDelta = V4f64(0, 0, 0, 0);
    v4f64 RDelta = V4f64(0, 0, 0, 0);
    if (Input->Forward)
    {
        TDelta.X += 1.0 / 64.0;
    }
    if (Input->Backward)
    {
        TDelta.X -= 1.0 / 64.0;
    }
    if (Input->Left)
    {
        TDelta.Y += 1.0 / 64.0;
    }
    if (Input->Right)
    {
        TDelta.Y -= 1.0 / 64.0;
    }
    if (Input->Up)
    {
        TDelta.Z += 1.0 / 64.0;
    }
    if (Input->Down)
    {
        TDelta.Z -= 1.0 / 64.0;
    }
    if (Input->RollLeft)
    {
        RDelta.X -= 1.0 / 64.0;
    }
    if (Input->RollRight)
    {
        RDelta.X += 1.0 / 64.0;
    }
    if (Input->PitchUp)
    {
        RDelta.Y -= 1.0 / 64.0;
    }
    if (Input->PitchDown)
    {
        RDelta.Y += 1.0 / 64.0;
    }
    if (Input->YawLeft)
    {
        RDelta.Z += 1.0 / 64.0;
    }
    if (Input->YawRight)
    {
        RDelta.Z -= 1.0 / 64.0;
    }
    CameraPosition = V4f64Add(CameraPosition, TDelta);
    CameraRotation = V4f64Add(CameraRotation, RDelta);

    DrawMesh(State, Mesh1, V2s32(600, 200));
    DrawMesh(State, Mesh2, V2s32(1400, 200));
    // DrawObject(State, Teapot);

    v4f64 CompassVertices[] = 
    {
        V4f64(0.0, 0.0, -0.5, 1.0), V4f64(4.0, 0.0, -0.5, 1.0), V4f64(0.0, 2.0, -0.5, 1.0),
        V4f64(0.5, 0.5, 0.5, 1.0), V4f64(2.0, 0.5, 0.5, 1.0), V4f64(0.5, 1.0, 0.5, 1.0),
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

    v4f64 SquareVertices[] =
    {
        V4f64(2.0, -2.0, -2.0, 1.0), V4f64(2.0, -2.0, 2.0, 1.0),
        V4f64(2.0, 2.0, 2.0, 1.0), V4f64(2.0, 2.0, -2.0, 1.0),
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
    // CameraPosition.X = 4.0;
    m4f64 CameraRotationMatrix = Model(V4f64(0.0, 0.0, 0.0, 0.0), CameraRotation, V4f64(1.0, 1.0, 1.0, 0.0));
    v4f64 CameraForward = M4f64MulV(CameraRotationMatrix, V4f64(1.0, 0.0, 0.0, 0.0));
    v4f64 CameraTarget = V4f64Add(CameraPosition, CameraForward);
    v4f64 CameraUp = M4f64MulV(CameraRotationMatrix, V4f64(0.0, 0.0, 1.0, 0.0));

    m4f64 M = Model(GlobalOffset, GlobalRotation, GlobalScale);
    m4f64 V = ViewMatrix(CameraPosition, CameraTarget, CameraUp);
    // m4f64 P = ProjectionOrtho(0.1, 10.0, -5.0, 5.0, -5.0, 5.0);
    m4f64 P = ProjectionPerspective(0.1, 5.0, (f64)State->Frame.Height / (f64)State->Frame.Width, 0.5 * PI);
    m4f64 MVP = M4f64Mul(P, M4f64Mul(V, M));

    State->MVP = MVP;

    // DrawObject3D(State, Square);
    // DrawObject3D(State, Compass);
    DrawObject3D(State, Teapot);
}
