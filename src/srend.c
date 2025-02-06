#include "srend.h"
#include "vector.c"

static u32 Offset = 0;

v4f64 Red = {1.0, 0.0, 0.0, 1.0};
v4f64 Green = {0.0, 1.0, 0.0, 1.0};
v4f64 Blue = {0.0, 0.0, 1.0, 1.0};
v4f64 White = {1.0, 1.0, 1.0, 1.0};

static void DrawScrollingGradient(frame *Frame)
{

    u8 Red = 0;

    for (u32 Y = 0; Y < Frame->Height; ++Y)
    {
        u8 Blue = (Y + Offset / 8) & 0xFF;
        for (u32 X = 0; X < Frame->Width; ++X)
        {
            u8 Green = (X + Offset / 8) & 0xFF;
            Frame->Pixels[Y * Frame->Width + X] = (Red << 16) | (Green << 8) | Blue;
        }
    }

}

static void DrawPixel(frame *Frame, s32 X, s32 Y, s32 C)
{
    s32 FrameIndex = Y * Frame->Width + X;
    Frame->Pixels[FrameIndex] = C;
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

static void DrawMesh(frame *Frame, v2s32 *Points, s32 PointCount, s32 (*Triangles)[3], s32 TriangleCount)
{
    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        Rasterize(Frame, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], White);
    }

    for (s32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        s32 *Triangle = Triangles[TriangleIndex];
        DrawTriangle(Frame, Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]], Red);
    }

    for (s32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
    {
        v2s32 Point = Points[PointIndex];
        DrawPoint(Frame, Point, Green);
    }
}

extern UPDATE_AND_DRAW(UpdateAndDraw)
{
    v4f64 Color = V4f64(1.0, (Offset & 0x0FFF) / (f64)0x0FFF, 0.0, 1.0);
    ++Offset;

    frame *Frame = &State->Frame;
    DrawScrollingGradient(Frame);

    DrawPoint(Frame, V2s32(30, 50), Color);
    DrawLine(Frame, V2s32(40, 20), V2s32(120, 80), Color);

    s32 ScanSize = 101;
    v2s32 ScanP0 = V2s32(20, 180);
    v2s32 ScanP1 = V2s32(ScanP0.X + ScanSize, ScanP0.Y + ScanSize);
    v2s32 ScanCenter = V2s32Div(V2s32Add(ScanP0, ScanP1), 2);
    
    DrawRect(Frame, V2s32(20, 180), V2s32(121, 281), Color);
    s32 ScanBorderIndex = (Offset / 50) % (4 * ScanSize);
    v2s32 ScanBorderPoint;
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

    Rasterize(Frame, Trig1[0], Trig1[1], Trig1[2], White);

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

    DrawMesh(Frame, Points, ArrayCount(Points), Triangles, ArrayCount(Triangles));

    DrawRect(Frame, V2s32(0, 0), V2s32(Frame->Width - 1, Frame->Height - 1), Color);
}

static inline s32 ToPixelColor(v4f64 Color)
{
    s32 Result = ((s8)(Color.A * 255.0) << 24) | ((s8)(Color.R * 255.0) << 16) | ((s8)(Color.G * 255.0) << 8) | ((s8)(Color.B * 255.0) << 0);
    return Result;
}
