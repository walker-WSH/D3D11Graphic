#pragma once
#include <Windows.h>
#include <DX11GraphicInstance.h>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

GRAPHIC_API void EnumGraphicCard();

GRAPHIC_API IDX11GraphicInstance *CreateGraphicInstance();
GRAPHIC_API void DestroyGraphicInstance(IDX11GraphicInstance *&graphic);
