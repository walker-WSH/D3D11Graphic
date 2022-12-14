#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <IDX11GraphicSession.h>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

#define COMBINE2(a, b) a##b
#define COMBINE1(a, b) COMBINE2(a, b)
#define AUTO_GRAPHIC_CONTEXT(graphic) \
	AutoGraphicContext COMBINE1(autoContext, __LINE__)(graphic, std::source_location::current())

GRAPHIC_API std::shared_ptr<std::vector<ST_GraphicCardInfo>> EnumGraphicCard();

GRAPHIC_API IDX11GraphicSession *CreateGraphicSession();
GRAPHIC_API void DestroyGraphicSession(IDX11GraphicSession *&graphic);

GRAPHIC_API void TransposeMatrixWVP(SIZE canvas, SIZE texture, RECT destPos, TextureRenderMode mode,
				    float outputMatrix[4][4]);
GRAPHIC_API void VertexList_RectTriangle(SIZE texture, bool flipH, bool flipV,
					 ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT]);
