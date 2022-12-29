#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <assert.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <source_location>
#include <optional>

#include "ComPtr.hpp"
#include "IDX11GraphicDefine.h"

#define SWAPCHAIN_TEXTURE_FORMAT DXGI_FORMAT_B8G8R8A8_UNORM
#define D2D_TEXTURE_FORMAT DXGI_FORMAT_B8G8R8A8_UNORM

static void CheckDXError(HRESULT hr,
			 std::source_location location = std::source_location::current())
{
#ifdef _DEBUG
	char str[MAX_PATH];
	snprintf(str, MAX_PATH, "==================COM ERROR 0X%x fun:%s line:%u file:%s \n", hr,
		 location.function_name(), location.line(), location.file_name());
	OutputDebugStringA(str);
#endif
}

const static std::vector<D3D_FEATURE_LEVEL> featureLevels = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

class DX11GraphicSession;
class DX11GraphicBase : public DX11GraphicObject {
public:
	DX11GraphicBase(DX11GraphicSession &graphic);
	virtual ~DX11GraphicBase();

	virtual const char *GetName() = 0;
	virtual bool BuildDX() = 0;
	virtual void ReleaseDX() = 0;

protected:
	DX11GraphicSession &m_graphic;
};
