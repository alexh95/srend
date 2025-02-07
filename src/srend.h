#ifndef SREND_H
#define SREND_H

#include "platform.h"
#include "string.h"
#include "vector.h"

extern RENDERER_INITIALIZE(RendererInitialize);
extern RENDERER_UPDATE_AND_DRAW(RendererUpdateAndDraw);

typedef struct
{
    v4f64 *Vertices;
    u32 VertexCount;
    u32 *Triangles;
    u32 TriangleCount;
    v4f64 Color;
} object;

#endif
