#include "DX11GraphicInstanceImpl.h"

#define CHECK_GRAPHIC_CONTEXT CheckContext(std::source_location::current())

DX11GraphicInstanceImpl::DX11GraphicInstanceImpl()
{
	InitializeCriticalSection(&m_lockOperation);
}

DX11GraphicInstanceImpl::~DX11GraphicInstanceImpl()
{
	DeleteCriticalSection(&m_lockOperation);
}

bool DX11GraphicInstanceImpl::InitializeGraphic(LUID luid)
{
	m_adapterLuid = luid;

	ReleaseDX();
	if (!BuildDX())
		return false;

	return false;
}

void DX11GraphicInstanceImpl::UnInitializeGraphic()
{
	ReleaseDX();

	// free all wrapper object
}

bool DX11GraphicInstanceImpl::BuildDX()
{
	ComPtr<IDXGIAdapter1> pAdapter;
	DXGraphic::EnumD3DAdapters(nullptr, [this, &pAdapter](void *userdata, ComPtr<IDXGIAdapter1> adapter, const DXGI_ADAPTER_DESC &desc,
							      const char *version) {
		if (desc.AdapterLuid.HighPart == m_adapterLuid.HighPart && desc.AdapterLuid.LowPart == m_adapterLuid.LowPart) {
			pAdapter = adapter;
			return false;
		}
		return true;
	});

	if (!pAdapter) {
		assert(false);
		return false;
	}

	D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_9_3;
	HRESULT hr = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels.data(),
				       (UINT)featureLevels.size(), D3D11_SDK_VERSION, m_pDX11Device.Assign(), &levelUsed,
				       m_pDeviceContext.Assign());

	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!InitBlendState())
		return false;

	if (!InitSamplerState())
		return false;

	return true;
}

bool DX11GraphicInstanceImpl::InitBlendState()
{
	D3D11_BLEND_DESC blendStateDescription = {};
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(m_pDX11Device->CreateBlendState(&blendStateDescription, m_pBlendState.Assign()))) {
		assert(false);
		return false;
	}

	return true;
}

bool DX11GraphicInstanceImpl::InitSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleState.Assign()))) {
		assert(false);
		return false;
	}

	return true;
}

void DX11GraphicInstanceImpl::ReleaseDX()
{
	m_pDX11Device = nullptr;
	m_pDeviceContext = nullptr;
	m_pBlendState = nullptr;
	m_pSampleState = nullptr;
}

void DX11GraphicInstanceImpl::RunTask1()
{
	CHECK_GRAPHIC_CONTEXT;

	// TODO
}
