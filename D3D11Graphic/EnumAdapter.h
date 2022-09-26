#pragma once
#include <Windows.h>
#include <DXDefine.h>
#include <ComPtr.hpp>
#include <functional>

namespace DXGraphic {
// callback: request stopping enum if callback returns false
void EnumD3DAdapters(void *userdata, std::function<bool(void *, ComPtr<IDXGIAdapter1>, const DXGI_ADAPTER_DESC &, const char *)> callback);

}
