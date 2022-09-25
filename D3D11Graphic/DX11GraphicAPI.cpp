#include "DX11GraphicAPI.h"
#include "DX11GraphicInstanceImpl.h"
#include <stack>
#include <mutex>
#include <assert.h>

GRAPHIC_API void EnumGraphicCard()
{
	// TODO
}

GRAPHIC_API IDX11GraphicInstance *CreateGraphicInstance()
{
	return new DX11GraphicInstanceImpl();
}

GRAPHIC_API void DestroyGraphicInstance(IDX11GraphicInstance *&graphic)
{
	if (graphic) {
		delete graphic;
		graphic = nullptr;
	}
}
