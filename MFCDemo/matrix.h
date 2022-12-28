#pragma once
#include <Windows.h>

enum class TextureRenderMode {
	FitToRect = 0,
	FullCoverRect,
};

void TransposeMatrixWVP(SIZE canvas, SIZE texture, RECT destPos, TextureRenderMode mode,
			float outputMatrix[4][4]);
