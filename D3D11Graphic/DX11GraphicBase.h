#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <assert.h>
#include <d3d11.h>
#include <dxgi.h>
#include <ComPtr.hpp>
#include <source_location>
#include <DX11GraphicDefine.h>

static void CheckDXError(HRESULT hr, std::source_location location = std::source_location::current())
{
	char str[MAX_PATH];
	snprintf(str, MAX_PATH, "================== 0X%x fun:%s line:%u file:%s \n", hr, location.function_name(), location.line(), location.file_name());
	OutputDebugStringA(str);
}

const static IID DXGIFactory2 = {0x50c83a1c, 0xe072, 0x4c48, {0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0}};
const static std::vector<D3D_FEATURE_LEVEL> featureLevels = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
};

class DX11GraphicInstanceImpl;
class DX11GraphicBase : public DX11GraphicObject {
public:
	DX11GraphicBase(DX11GraphicInstanceImpl &graphic);
	virtual ~DX11GraphicBase();

	virtual bool BuildDX() = 0;
	virtual void ReleaseDX() = 0;

protected:
	DX11GraphicInstanceImpl &m_graphic;
};
