#include "srend.h"
#include "vector.c"

static u32 Offset = 0;

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
        SWAP(s32, X0, X1);
        SWAP(s32, Y0, Y1);
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
        SWAP(s32, X0, X1);
        SWAP(s32, Y0, Y1);
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

    DrawTriangle(Frame, V2s32(300, 15), V2s32(380, 30), V2s32(350, 80), Color);

    DrawRect(Frame, V2s32(0, 0), V2s32(Frame->Width - 1, Frame->Height - 1), Color);
}

static inline s32 ToPixelColor(v4f64 Color)
{
    s32 Result = ((s8)(Color.A * 255.0) << 24) | ((s8)(Color.R * 255.0) << 16) | ((s8)(Color.G * 255.0) << 8) | ((s8)(Color.B * 255.0) << 0);
    return Result;
}
