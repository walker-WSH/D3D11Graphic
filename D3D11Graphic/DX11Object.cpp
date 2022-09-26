#include "DX11Object.h"
#include <DX11GraphicInstanceImpl.h>

DX11Object::DX11Object(DX11GraphicInstanceImpl &graphic) : m_graphic(graphic)
{
	m_graphic.PushObject(this);
}

DX11Object::~DX11Object()
{
	m_graphic.RemoveObject(this);
}
