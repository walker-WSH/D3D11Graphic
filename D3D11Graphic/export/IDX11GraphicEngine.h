#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <IDX11GraphicSession.h>

GRAPHIC_API std::shared_ptr<std::vector<ST_GraphicCardInfo>> EnumGraphicCard();

GRAPHIC_API IDX11GraphicSession *CreateGraphicSession();
GRAPHIC_API void DestroyGraphicSession(IDX11GraphicSession *&graphic);
