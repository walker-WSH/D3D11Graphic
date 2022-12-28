#pragma once
#include <Windows.h>

#define TEXTURE_VERTEX_COUNT 4

struct ST_TextureVertex {
	float x, y, z, w;
	float u, v;
};

void FillTextureVertex(SIZE texture, bool flipH, bool flipV, float cropLeft, float cropTop,
			     float cropRight, float cropBtm,
			     ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT]);
