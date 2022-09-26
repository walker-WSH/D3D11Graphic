#include "DX11GraphicBase.h"
#include <DX11GraphicInstanceImpl.h>

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")

DX11GraphicBase::DX11GraphicBase(DX11GraphicInstanceImpl &graphic) : m_graphic(graphic)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.PushObject(this);
}

DX11GraphicBase::~DX11GraphicBase()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.RemoveObject(this);
}
