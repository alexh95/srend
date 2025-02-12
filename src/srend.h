#ifndef SREND_H
#define SREND_H

#include "platform.h"
#include "string.h"
#include "vector.h"
#include "matrix.h"

extern RENDERER_INITIALIZE(RendererInitialize);
extern RENDERER_UPDATE_AND_DRAW(RendererUpdateAndDraw);

typedef struct
{
    v4 *Vertices;
    u32 VertexCount;
    u32 *Triangles;
    u32 TriangleCount;
    v4 Color;
} object;

#endif
