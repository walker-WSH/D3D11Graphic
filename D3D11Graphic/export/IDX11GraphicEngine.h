#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <IDX11GraphicSession.h>

GRAPHIC_API std::shared_ptr<std::vector<ST_GraphicCardInfo>> EnumGraphicCard();

GRAPHIC_API IDX11GraphicSession *CreateGraphicSession();
GRAPHIC_API void DestroyGraphicSession(IDX11GraphicSession *&graphic);

GRAPHIC_API void TransposeMatrixWVP(SIZE canvas, SIZE texture, RECT destPos, TextureRenderMode mode,
				    float outputMatrix[4][4]);
GRAPHIC_API void VertexList_RectTriangle(SIZE texture, bool flipH, bool flipV,
					 ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT]);
