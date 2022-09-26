#include "DX11GraphicAPI.h"
#include "DX11GraphicInstanceImpl.h"
#include "EnumAdapter.h"
#include <stack>
#include <mutex>
#include <assert.h>

GRAPHIC_API std::shared_ptr<std::vector<ST_GraphicCardInfo>> EnumGraphicCard()
{
	std::shared_ptr<std::vector<ST_GraphicCardInfo>> pReturnList(new std::vector<ST_GraphicCardInfo>,
								  [](std::vector<ST_GraphicCardInfo> *ptr) { delete ptr; });

	DXGraphic::EnumD3DAdapters(nullptr, [pReturnList](void *userdata, ComPtr<IDXGIAdapter1> adapter, const DXGI_ADAPTER_DESC &desc,
							  const char *version) {
		ST_GraphicCardInfo info;
		info.Name = desc.Description;
		info.Driver = version;

		info.AdapterLuid = desc.AdapterLuid;
		info.VendorId = desc.VendorId;
		info.DeviceId = desc.DeviceId;

		info.DedicatedVideoMemory = desc.DedicatedVideoMemory;
		info.DedicatedSystemMemory = desc.DedicatedSystemMemory;
		info.SharedSystemMemory = desc.SharedSystemMemory;

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
