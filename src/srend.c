#include "srend.h"

extern UPDATE_AND_DRAW(UpdateAndDraw)
{
    frame *Frame = &State->Frame;
    static u32 Offset = 0;

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

    ++Offset;
}
