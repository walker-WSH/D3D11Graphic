#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <DXDefine.h>

class DX11GraphicInstanceImpl;
class DX11Object {
public:
	DX11Object(DX11GraphicInstanceImpl &graphic);
	virtual ~DX11Object();

	virtual bool BuildDX() = 0;
	virtual void ReleaseDX() = 0;
	virtual bool IsBuilt() = 0;

protected:
	DX11GraphicInstanceImpl &m_graphic;
};
