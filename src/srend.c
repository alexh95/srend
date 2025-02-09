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

static inline s32 ToPixelColor(v4f64 Color)
{
    s32 Result = ((s8)(Color.A * 255.0) << 24) | ((s8)(Color.R * 255.0) << 16) | ((s8)(Color.G * 255.0) << 8) | ((s8)(Color.B * 255.0) << 0);
    return Result;
}

static void DrawScrollingGradient(frame *Frame)
{
    u8 Red = 0;

    for (u32 Y = 0; Y < Frame->Height; ++Y)
    {
        u8 Blue = (Y + GlobalFrameCounter / 8) & 0xFF;
        for (u32 X = 0; X < Frame->Width; ++X)
        {
            u8 Green = (X + GlobalFrameCounter / 8) & 0xFF;
            Frame->Pixels[Y * Frame->Width + X] = (Red << 16) | (Green << 8) | Blue;
        }
    }

}

static void DrawPixel(frame *Frame, s32 X, s32 Y, s32 C)
{
    if (X >= 0 && X < (s32)Frame->Width && Y >= 0 && Y < (s32)Frame->Height)
    {
        s32 FrameIndex = Y * Frame->Width + X;
        Frame->Pixels[FrameIndex] = C;
    }
}

static void DrawPoint(frame *Frame, v2s32 P, v4f64 Color)
{

    s32 C = ToPixelColor(Color);
    DrawPixel(Frame, P.X, P.Y, C);
}

static void DrawLineH(frame* Frame, s32 X0, s32 Y0, s32 X1, s32 Y1, s32 C)
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
            DrawPixel(Frame, X, Y, C);
            if (DecisionValue >= 0)
            {
                Y += Direction;
                DecisionValue -= 2 * DX;
            }
            DecisionValue += 2 * DY;
        }
    }
}

static void DrawLineV(frame* Frame, s32 X0, s32 Y0, s32 X1, s32 Y1, s32 C)
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
            DrawPixel(Frame, X, Y, C);
            if (DecisionValue >= 0)
            {
                X += Direction;
                DecisionValue -= 2 * DY;
            }
            DecisionValue += 2 * DX;
        }
    }
}

static void DrawLine(frame* Frame, v2s32 P0, v2s32 P1, v4f64 Color)
{
    s32 C = ToPixelColor(Color);
    if (ABS(P1.X - P0.X) > ABS(P1.Y - P0.Y))
    {
        DrawLineH(Frame, P0.X, P0.Y, P1.X, P1.Y, C);
    }
    else
    {
        DrawLineV(Frame, P0.X, P0.Y, P1.X, P1.Y, C);
    }
}

static void DrawRect(frame *Frame, v2s32 P0, v2s32 P1, v4f64 Color)
{
    DrawLine(Frame, V2s32(P0.X, P0.Y), V2s32(P1.X, P0.Y), Color);
    DrawLine(Frame, V2s32(P1.X, P0.Y), V2s32(P1.X, P1.Y), Color);
    DrawLine(Frame, V2s32(P1.X, P1.Y), V2s32(P0.X, P1.Y), Color);
    DrawLine(Frame, V2s32(P0.X, P1.Y), V2s32(P0.X, P0.Y), Color);
}

static void DrawTriangle(frame *Frame, v2s32 P0, v2s32 P1, v2s32 P2, v4f64 Color)
{
    DrawLine(Frame, P0, P1, Color);
    DrawLine(Frame, P1, P2, Color);
    DrawLine(Frame, P2, P0, Color);
}

static s32 EdgeFunction(v2s32 P0, v2s32 P1, v2s32 P2)
{
    s32 Result = (P1.X - P0.X) * (P2.Y - P0.Y) - (P1.Y - P0.Y) * (P2.X - P0.X);
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

static void Rasterize(frame *Frame, v2s32 P0, v2s32 P1, v2s32 P2, v4f64 C)
{
    s32 MinX = MIN3(P0.X, P1.X, P2.X);
    s32 MaxX = MAX3(P0.X, P1.X, P2.X);
    s32 MinY = MIN3(P0.Y, P1.Y, P2.Y);
    s32 MaxY = MAX3(P0.Y, P1.Y, P2.Y);

    for (s32 Y = MinY; Y <= MaxY; ++Y)
    {
        for (s32 X = MinX; X <= MaxX; ++X)
        {
            v2s32 P = V2s32(X, Y);
            b32 Inside = PointInside(P, P0, P1, P2);
            if (Inside)
            {
                DrawPoint(Frame, P, C);
            }
        }
    }
}

static void DrawMesh_(frame *Frame, v2s32 *Points, s32 PointCount, s32 (*Triangles)[3], s32 TriangleCount)
{
    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        Rasterize(Frame, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalWhite);
    }

    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        DrawTriangle(Frame, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], GlobalRed);
    }

    for (s32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
    {
        v2s32 Point = Points[PointIndex];
        DrawPoint(Frame, Point, GlobalGreen);
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

static void DrawMesh(frame *Frame, object Mesh, v2s32 Offset)
{
    for (u32 TriangleIndex = 0; TriangleIndex < Mesh.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Mesh.Triangles + 3 * TriangleIndex;
        Rasterize(
            Frame, 
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
            Frame, 
            ToScreenSpace(Mesh.Vertices[Triangle[0]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[1]], Offset), 
            ToScreenSpace(Mesh.Vertices[Triangle[2]], Offset), 
            GlobalRed
        );
    }

    for (u32 PointIndex = 0; PointIndex < Mesh.VertexCount; ++PointIndex)
    {
        v2s32 Point = ToScreenSpace(Mesh.Vertices[PointIndex], Offset);
        DrawPoint(Frame, Point, GlobalGreen);
    }
}

static void DrawObject(frame *Frame, object Object)
{
    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;
        Rasterize(
            Frame, 
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
            Frame, 
            ToScreenSpaceObj(Object.Vertices[Triangle[0]]), 
            ToScreenSpaceObj(Object.Vertices[Triangle[1]]), 
            ToScreenSpaceObj(Object.Vertices[Triangle[2]]), 
            GlobalRed
        );
    }

    for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
    {
        v2s32 Point = ToScreenSpaceObj(Object.Vertices[PointIndex]);
        DrawPoint(Frame, Point, GlobalGreen);
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

static f64 Clamp(f64 V, f64 V0, f64 V1)
{
    if (V0 < V)
    {
        return V0;
    }
    else if (V > V1)
    {
        return V1;
    }
    return V;
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

static v4f64 NormalToScreenSpace(frame* Frame, v4f64 N)
{
    v4f64 Result = N;

    if (N.W != 0.0)
    {
        f64 W = ABS(Result.W);
        Result.X /= W;
        Result.Y /= W;
        Result.Z /= W;
    }

    Result.Y = 0.5 * (1.0 - Result.Y) * (f64)Frame->Width;
    Result.Z = 0.5 * (1.0 + Result.Z) * (f64)Frame->Height;

    return Result;
}

static v2s32 ToScreen(v4f64 S)
{
    v2s32 Result = V2s32((s32)S.Y, (s32)S.Z);
    return Result;
}

static void DrawObject3D(frame *Frame, object Object)
{
    m4f64 M = Model(GlobalOffset, GlobalRotation, GlobalScale);
    m4f64 V = M4f64I();
    // m4f64 P = ProjectionOrtho(0.1, 5.0, -5.0, 5.0, -5.0, 5.0);
    m4f64 P = ProjectionPerspective(0.1, 100.0, (f64)Frame->Height / (f64)Frame->Width, 0.5 * PI);
    m4f64 MVP = M4f64Mul(P, M4f64Mul(V, M));

    // for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    // {
    //     u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

    //     v4f64 W0 = Object.Vertices[Triangle[0]];
    //     v4f64 W1 = Object.Vertices[Triangle[1]];
    //     v4f64 W2 = Object.Vertices[Triangle[2]];

    //     v4f64 N0 = M4f64MulV(MVP, W0);
    //     v4f64 N1 = M4f64MulV(MVP, W1);
    //     v4f64 N2 = M4f64MulV(MVP, W2);

    //     v4f64 S0 = NormalToScreenSpace(Frame, N0);
    //     v4f64 S1 = NormalToScreenSpace(Frame, N1);
    //     v4f64 S2 = NormalToScreenSpace(Frame, N2);

    //     if (S0.X >= 0.0 && S0.X <= 1.0 && S1.X >= 0.0 && S1.X <= 1.0 && S2.X >= 0.0 && S2.X <= 1.0)
    //     {
    //         Rasterize(
    //             Frame, 
    //             ToScreen(S0), 
    //             ToScreen(S1), 
    //             ToScreen(S2), 
    //             Object.Color
    //         );
    //     }
    // }

    for (u32 TriangleIndex = 0; TriangleIndex < Object.TriangleCount; ++TriangleIndex)
    {
        u32 *Triangle = Object.Triangles + 3 * TriangleIndex;

        v4f64 W0 = Object.Vertices[Triangle[0]];
        v4f64 W1 = Object.Vertices[Triangle[1]];
        v4f64 W2 = Object.Vertices[Triangle[2]];

        v4f64 N0 = M4f64MulV(MVP, W0);
        v4f64 N1 = M4f64MulV(MVP, W1);
        v4f64 N2 = M4f64MulV(MVP, W2);

        v4f64 S0 = NormalToScreenSpace(Frame, N0);
        v4f64 S1 = NormalToScreenSpace(Frame, N1);
        v4f64 S2 = NormalToScreenSpace(Frame, N2);

        if (S0.X >= 0.0 && S0.X <= 1.0 && S1.X >= 0.0 && S1.X <= 1.0 && S2.X >= 0.0 && S2.X <= 1.0)
        {
            DrawTriangle(
                Frame, 
                ToScreen(S0), 
                ToScreen(S1), 
                ToScreen(S2), 
                GlobalRed
            );
        }
    }

    // for (u32 PointIndex = 0; PointIndex < Object.VertexCount; ++PointIndex)
    // {
    //     v2s32 Point = ToScreenSpaceObj(Object.Vertices[PointIndex]);
    //     DrawPoint(Frame, Point, Green);
    // }
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

extern RENDERER_UPDATE_AND_DRAW(RendererUpdateAndDraw)
{
    v4f64 Color = V4f64(1.0, (GlobalFrameCounter & 0x0FFF) / (f64)0x0FFF, 0.0, 1.0);
    ++GlobalFrameCounter;

    frame *Frame = &State->Frame;
    DrawScrollingGradient(Frame);

    DrawPoint(Frame, V2s32(30, 50), Color);
    DrawLine(Frame, V2s32(40, 20), V2s32(120, 80), Color);

    s32 ScanSize = 101;
    v2s32 ScanP0 = V2s32(20, 180);
    v2s32 ScanP1 = V2s32(ScanP0.X + ScanSize, ScanP0.Y + ScanSize);
    v2s32 ScanCenter = V2s32Div(V2s32Add(ScanP0, ScanP1), 2);
    
    DrawRect(Frame, V2s32(20, 180), V2s32(121, 281), Color);
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
    DrawLine(Frame, ScanCenter, ScanBorderPoint, Color);

    v2s32 Trig0[] = { V2s32(300, 15), V2s32(380, 30), V2s32(350, 80) };
    v2s32 Trig1[] = { V2s32(350, 100), V2s32(270, 200), V2s32(200, 100) };

    s32 T0 = EdgeFunction(Trig0[2], Trig0[1], Trig0[0]);
    s32 T1 = EdgeFunction(Trig1[0], Trig1[1], Trig1[2]);

    Rasterize(Frame, Trig1[0], Trig1[1], Trig1[2], GlobalWhite);

    DrawTriangle(Frame, Trig0[0], Trig0[1], Trig0[2], Color);
    DrawTriangle(Frame, Trig1[0], Trig1[1], Trig1[2], Color);

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

    DrawMesh_(Frame, Points, ArrayCount(Points), Triangles, ArrayCount(Triangles));

    DrawRect(Frame, V2s32(0, 0), V2s32(Frame->Width - 1, Frame->Height - 1), Color);

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
    GlobalOffset = V4f64Add(GlobalOffset, TDelta);
    GlobalRotation = V4f64Add(GlobalRotation, RDelta);

    DrawMesh(Frame, Mesh1, V2s32(600, 200));
    DrawMesh(Frame, Mesh2, V2s32(1400, 200));
    // DrawObject(Frame, Teapot);

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

    object Square;
    Square.Vertices = SquareVertices;
    Square.VertexCount = ArrayCount(SquareVertices);
    Square.Triangles = SquareTriangles;
    Square.TriangleCount = ArrayCount(SquareTriangles) / 3;

    // DrawObject3D(Frame, Square);
    // DrawObject3D(Frame, Compass);
    DrawObject3D(Frame, Teapot);
}
