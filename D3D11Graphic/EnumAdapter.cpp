#include "EnumAdapter.h"
#include <algorithm>

namespace DXGraphic {
static uint32_t vendoridMicrosoft = 0x1414;
static uint32_t vendoridIntel = 0x8086;
static uint32_t vendoridNvidia = 0x10DE;
static uint32_t vendoridAMD = 0x1002;

std::vector<std::wstring> wstrINTEL = {L"INTEL"};
std::vector<std::wstring> wstrNVIDIA = {L"NVIDIA"};
std::vector<std::wstring> wstrAMD = {L"RADEON", L"AMD"};

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

bool MatchString(const std::wstring &desc, const std::vector<std::wstring> &keyList)
{
	for (const auto &key : keyList) {
		if (desc.find(key) != std::wstring::npos)
			return true;
	}

	return false;
}

enum GraphicCardType CheckAdapterType(const DXGI_ADAPTER_DESC &desc)
{
	if (desc.VendorId == vendoridMicrosoft && desc.DeviceId == 0x8c)
		return GraphicCardType::msbasic;

	std::wstring wstr(desc.Description);
	transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);

	if (MatchString(wstr, wstrNVIDIA) || vendoridNvidia == desc.VendorId)
		return GraphicCardType::nvidia;

	if (MatchString(wstr, wstrAMD) || vendoridAMD == desc.VendorId)
		return GraphicCardType::amd;

	if (MatchString(wstr, wstrINTEL) || vendoridIntel == desc.VendorId)
		return GraphicCardType::intel;

	return GraphicCardType::any;
}
}
