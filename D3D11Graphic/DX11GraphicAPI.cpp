#include "DX11GraphicAPI.h"
#include "DX11GraphicInstanceImpl.h"
#include "EnumAdapter.h"
#include <stack>
#include <mutex>
#include <assert.h>

GRAPHIC_API std::shared_ptr<std::vector<ST_GraphicCardInfo>> EnumGraphicCard()
{
	std::shared_ptr<std::vector<ST_GraphicCardInfo>> pReturnList(
		new std::vector<ST_GraphicCardInfo>,
		[](std::vector<ST_GraphicCardInfo> *ptr) { delete ptr; });

	DXGraphic::EnumD3DAdapters(nullptr, [pReturnList](void *userdata, ComPtr<IDXGIFactory1>,
							  ComPtr<IDXGIAdapter1> adapter,
							  const DXGI_ADAPTER_DESC &desc,
							  const char *version) {
		ST_GraphicCardInfo info;
		info.graphicName = desc.Description;
		info.driverVersion = version;

		info.adapterLuid = desc.AdapterLuid;
		info.vendorId = desc.VendorId;
		info.deviceId = desc.DeviceId;

		info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
		info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
		info.sharedSystemMemory = desc.SharedSystemMemory;

		pReturnList->push_back(info);
		return true;
	});

	return pReturnList;
}

GRAPHIC_API IDX11GraphicInstance *CreateGraphicInstance()
{
	return new DX11GraphicInstanceImpl();
}

GRAPHIC_API void DestroyGraphicInstance(IDX11GraphicInstance *&graphic)
{
	if (graphic) {
		delete graphic;
		graphic = nullptr;
	}
}
