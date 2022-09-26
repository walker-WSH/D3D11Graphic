#include "DX11Object.h"
#include <DX11GraphicInstanceImpl.h>

DX11Object::DX11Object(DX11GraphicInstanceImpl &graphic) : m_graphic(graphic)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.PushObject(this);
}

DX11Object::~DX11Object()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	m_graphic.RemoveObject(this);
}
