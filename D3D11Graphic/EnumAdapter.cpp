#include "EnumAdapter.h"

namespace DXGraphic {
void EnumD3DAdapters(void *userdata,
		     std::function<bool(void *, ComPtr<IDXGIFactory1>, ComPtr<IDXGIAdapter1>, const DXGI_ADAPTER_DESC &, const char *)> callback)
{
	ComPtr<IDXGIFactory1> factory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		assert(false);
		return;
	}

	ComPtr<IDXGIAdapter1> adapter;
	for (uint32_t i = 0; factory->EnumAdapters1(i, adapter.Assign()) == S_OK; ++i) {
		DXGI_ADAPTER_DESC desc;
		hr = adapter->GetDesc(&desc);
		if (FAILED(hr))
			continue;

		/* ignore Microsoft's 'basic' renderer' */
		if (desc.VendorId == 0x1414 && desc.DeviceId == 0x8c)
			continue;

		char versionStr[MAX_PATH] = {0};
		LARGE_INTEGER versionNum;
		hr = adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &versionNum);
		if (SUCCEEDED(hr)) {
			snprintf(versionStr, MAX_PATH, "%d.%d.%d.%d", HIWORD(versionNum.HighPart), LOWORD(versionNum.HighPart), HIWORD(versionNum.LowPart),
				 LOWORD(versionNum.LowPart));
		}

		if (!callback(userdata, factory, adapter, desc, versionStr))
			break;
	}
}

}
