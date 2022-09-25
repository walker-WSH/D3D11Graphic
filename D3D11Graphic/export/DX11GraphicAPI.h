#pragma once
#include <Windows.h>
#include <DX11GraphicInstance.h>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

#define COMBINE2(a, b) a##b
#define COMBINE1(a, b) COMBINE2(a, b)
#define AUTO_GRAPHIC_CONTEXT(graphic) AutoGraphicContext COMBINE1(autoContext, __LINE__)(graphic, std::source_location::current())

GRAPHIC_API void EnumGraphicCard();
GRAPHIC_API IDX11GraphicInstance *CreateGraphicInstance();
GRAPHIC_API void DestroyGraphicInstance(IDX11GraphicInstance *&graphic);
