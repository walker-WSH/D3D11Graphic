#pragma once
#include <Windows.h>
#include <DX11GraphicBase.h>
#include <ComPtr.hpp>
#include <functional>
#include <map>

enum class GraphicCardType {
	any = 0,
	nvidia,
	amd,
	intel,
	msbasic,
};

static std::map<GraphicCardType, int> g_mapGraphicOrder = {
	{GraphicCardType::nvidia, 1},  {GraphicCardType::amd, 2}, {GraphicCardType::intel, 3},
	{GraphicCardType::msbasic, 4}, {GraphicCardType::any, 5},
};

namespace DXGraphic {
// callback: request stopping enum if callback returns false
void EnumD3DAdapters(void *userdata,
		     std::function<bool(void *, ComPtr<IDXGIFactory1> factory, ComPtr<IDXGIAdapter1>,
					const DXGI_ADAPTER_DESC &, const char *)>
			     callback);

enum GraphicCardType CheckAdapterType(const DXGI_ADAPTER_DESC &desc);
}
