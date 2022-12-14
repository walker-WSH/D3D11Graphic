#include "DX11GraphicBase.h"
#include <DX11GraphicSession.h>

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// dxsdk
#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "D3DX10.lib")

DX11GraphicBase::DX11GraphicBase(DX11GraphicSession &graphic) : m_graphic(graphic)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.PushObject(this);
}

DX11GraphicBase::~DX11GraphicBase()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.RemoveObject(this);
}
