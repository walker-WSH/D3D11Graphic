#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

GRAPHIC_API void EnumGraphicCard();