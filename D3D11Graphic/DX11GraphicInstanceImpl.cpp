#include "DX11GraphicInstanceImpl.h"
#include "DX11Texture2D.h"

HMODULE DX11GraphicInstanceImpl::s_hDllModule = nullptr;

DX11GraphicInstanceImpl::DX11GraphicInstanceImpl()
{
	InitializeCriticalSection(&m_lockOperation);
}

DX11GraphicInstanceImpl::~DX11GraphicInstanceImpl()
{
	assert(m_listObject.empty());
	DeleteCriticalSection(&m_lockOperation);
}

bool DX11GraphicInstanceImpl::InitializeGraphic(LUID luid)
{
	CHECK_GRAPHIC_CONTEXT;

	m_adapterLuid = luid;

	m_pShaderDefault = std::shared_ptr<DX11ShaderTexture>(new DX11ShaderTexture(*this, L"defaultVS.cso", L"defaultPS.cso", sizeof(float) * 16, 0));

	m_pShaderBorder = std::shared_ptr<DX11ShaderTexture>(new DX11ShaderTexture(*this, L"borderVS.cso", L"borderPS.cso", sizeof(float) * 16, 0));

	ReleaseDX();
	if (!BuildDX())
		return false;

	return false;
}

void DX11GraphicInstanceImpl::UnInitializeGraphic()
{
	CHECK_GRAPHIC_CONTEXT;

	ReleaseDX();

	m_pShaderDefault = nullptr;
	m_pShaderBorder = nullptr;

	assert(m_listObject.empty());
	for (auto &item : m_listObject)
		delete item;
}

void DX11GraphicInstanceImpl::ReleaseGraphicObject(void *&hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	auto obj = static_cast<DX11Object *>(hdl);
	obj->ReleaseDX();
	delete obj;
}

texture_handle DX11GraphicInstanceImpl::OpenTexture(HANDLE hSharedHanle)
{
	CHECK_GRAPHIC_CONTEXT;
	return new DX11Texture2D(*this, hSharedHanle);
}

texture_handle DX11GraphicInstanceImpl::CreateReadTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format)
{
	CHECK_GRAPHIC_CONTEXT;
	return new DX11Texture2D(*this, width, height, format, TextureType::ReadTexture);
}

texture_handle DX11GraphicInstanceImpl::CreateWriteTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format)
{
	CHECK_GRAPHIC_CONTEXT;
	return new DX11Texture2D(*this, width, height, format, TextureType::WriteTexture);
}

texture_handle DX11GraphicInstanceImpl::CreateRenderTarget(uint32_t width, uint32_t height, enum DXGI_FORMAT format)
{
	CHECK_GRAPHIC_CONTEXT;
	return new DX11Texture2D(*this, width, height, format, TextureType::RenderTarget);
}

ST_TextureInfo DX11GraphicInstanceImpl::GetTextureInfo(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;

	auto obj = static_cast<DX11Texture2D *>(hdl);
	if (!obj->m_pTexture2D)
		return ST_TextureInfo();

	return ST_TextureInfo(obj->m_descTexture.Width, obj->m_descTexture.Height, obj->m_descTexture.Format);
}

void DX11GraphicInstanceImpl::ReleaseDX()
{
	CHECK_GRAPHIC_CONTEXT;

	for (auto &item : m_listObject)
		item->ReleaseDX();

	m_pDX11Device = nullptr;
	m_pDeviceContext = nullptr;
	m_pBlendState = nullptr;
	m_pSampleState = nullptr;
}

bool DX11GraphicInstanceImpl::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT;

	ComPtr<IDXGIAdapter1> pAdapter;
	DXGraphic::EnumD3DAdapters(nullptr,
				   [this, &pAdapter](void *userdata, ComPtr<IDXGIAdapter1> adapter, const DXGI_ADAPTER_DESC &desc, const char *version) {
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
				       (UINT)featureLevels.size(), D3D11_SDK_VERSION, m_pDX11Device.Assign(), &levelUsed, m_pDeviceContext.Assign());

	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!InitBlendState())
		return false;

	if (!InitSamplerState())
		return false;

	for (auto &item : m_listObject)
		item->BuildDX();

	return true;
}

bool DX11GraphicInstanceImpl::InitBlendState()
{
	CHECK_GRAPHIC_CONTEXT;

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
	CHECK_GRAPHIC_CONTEXT;

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

ComPtr<ID3D11Device> DX11GraphicInstanceImpl::DXDevice()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Device;
}

ComPtr<ID3D11DeviceContext> DX11GraphicInstanceImpl::DXContext()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDeviceContext;
}

void DX11GraphicInstanceImpl::RemoveObject(DX11Object *obj)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = find(m_listObject.begin(), m_listObject.end(), obj);
	if (itr != m_listObject.end())
		m_listObject.erase(itr);
}

void DX11GraphicInstanceImpl::PushObject(DX11Object *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	m_listObject.push_back(obj);
}
