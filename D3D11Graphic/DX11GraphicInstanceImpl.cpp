#include "DX11GraphicInstanceImpl.h"

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

	return BuildAllDX();
}

void DX11GraphicInstanceImpl::UnInitializeGraphic()
{
	CHECK_GRAPHIC_CONTEXT;

	ReleaseAllDX();

	assert(m_listObject.empty());
	for (auto &item : m_listObject)
		delete item;
}

void DX11GraphicInstanceImpl::ReleaseGraphicObject(DX11GraphicObject *&hdl)
{
	CHECK_GRAPHIC_CONTEXT;

	auto obj = dynamic_cast<DX11GraphicBase *>(hdl);
	assert(obj);
	if (obj)
		obj->ReleaseDX();

	delete hdl;
	hdl = nullptr;
}

texture_handle DX11GraphicInstanceImpl::OpenSharedTexture(HANDLE hSharedHanle)
{
	CHECK_GRAPHIC_CONTEXT;

	DX11Texture2D *tex = new DX11Texture2D(*this, hSharedHanle);
	if (!tex->IsBuilt()) {
		delete tex;
		assert(false);
		return nullptr;
	}

	return tex;
}

texture_handle DX11GraphicInstanceImpl::OpenImageTexture(const WCHAR *fullPath)
{
	CHECK_GRAPHIC_CONTEXT;

	DX11Texture2D *tex = new DX11Texture2D(*this, fullPath);
	if (!tex->IsBuilt()) {
		delete tex;
		assert(false);
		return nullptr;
	}

	return tex;
}

texture_handle DX11GraphicInstanceImpl::CreateTexture(const ST_TextureInfo &info)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(TextureType::SharedHandle != info.usage);
	assert(TextureType::StaticImageFile != info.usage);

	DX11Texture2D *tex = new DX11Texture2D(*this, info);
	if (!tex->IsBuilt()) {
		delete tex;
		assert(false);
		return nullptr;
	}

	return tex;
}

ST_TextureInfo DX11GraphicInstanceImpl::GetTextureInfo(texture_handle tex)
{
	CHECK_GRAPHIC_CONTEXT;

	auto obj = dynamic_cast<DX11Texture2D *>(tex);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return ST_TextureInfo();

	return ST_TextureInfo(obj->m_descTexture.Width, obj->m_descTexture.Height, obj->m_descTexture.Format, obj->m_textureInfo.usage);
}

bool DX11GraphicInstanceImpl::CopyTexture(texture_handle dest, texture_handle src)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_bBuildSuccessed)
		return false;

	auto destTex = dynamic_cast<DX11Texture2D *>(dest);
	assert(destTex);
	if (!destTex || !destTex->IsBuilt())
		return false;

	auto srcTex = dynamic_cast<DX11Texture2D *>(src);
	assert(srcTex);
	if (!srcTex || !srcTex->IsBuilt())
		return false;

	assert(destTex->m_descTexture.Width == srcTex->m_descTexture.Width);
	assert(destTex->m_descTexture.Height == srcTex->m_descTexture.Height);
	assert(destTex->m_descTexture.Format == srcTex->m_descTexture.Format);

	m_pDeviceContext->CopyResource(destTex->m_pTexture2D, srcTex->m_pTexture2D);
	return true;
}

bool DX11GraphicInstanceImpl::MapTexture(texture_handle tex, bool isRead, D3D11_MAPPED_SUBRESOURCE *mapData)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_bBuildSuccessed)
		return false;

	auto obj = dynamic_cast<DX11Texture2D *>(tex);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return false;

	D3D11_MAP type = isRead ? D3D11_MAP_READ : D3D11_MAP_WRITE_DISCARD;
	HRESULT hr = m_pDeviceContext->Map(obj->m_pTexture2D, 0, type, 0, mapData);
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}

void DX11GraphicInstanceImpl::UnmapTexture(texture_handle tex)
{
	CHECK_GRAPHIC_CONTEXT;

	auto obj = dynamic_cast<DX11Texture2D *>(tex);
	assert(obj);
	if (!obj || !obj->IsBuilt()) {
		assert(false);
		return;
	}

	m_pDeviceContext->Unmap(obj->m_pTexture2D, 0);
}

display_handle DX11GraphicInstanceImpl::CreateDisplay(HWND hWnd)
{
	CHECK_GRAPHIC_CONTEXT;

	auto ret = new DX11SwapChain(*this, hWnd);
	if (!ret->IsBuilt()) {
		delete ret;
		assert(false);
		return nullptr;
	}

	return ret;
}

void DX11GraphicInstanceImpl::SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height)
{
	auto obj = dynamic_cast<DX11SwapChain *>(hdl);
	assert(obj);
	if (obj)
		obj->SetDisplaySize(width, height);
}

shader_handle DX11GraphicInstanceImpl::CreateShader(const ST_ShaderInfo &info)
{
	CHECK_GRAPHIC_CONTEXT;

	auto ret = new DX11ShaderTexture(*this, &info);
	if (!ret->IsBuilt()) {
		delete ret;
		assert(false);
		return nullptr;
	}

	return ret;
}

ComPtr<IDXGIFactory1> DX11GraphicInstanceImpl::DXFactory()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Factory;
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

void DX11GraphicInstanceImpl::PushObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	m_listObject.push_back(obj);
}

void DX11GraphicInstanceImpl::RemoveObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = find(m_listObject.begin(), m_listObject.end(), obj);
	if (itr != m_listObject.end())
		m_listObject.erase(itr);
}

void DX11GraphicInstanceImpl::ReleaseAllDX()
{
	CHECK_GRAPHIC_CONTEXT;

	m_bBuildSuccessed = false;

	for (auto &item : m_listObject)
		item->ReleaseDX();

	m_pDX11Factory = nullptr;
	m_pDX11Device = nullptr;
	m_pDeviceContext = nullptr;
	m_pBlendState = nullptr;
	m_pSampleState = nullptr;
}

bool DX11GraphicInstanceImpl::BuildAllDX()
{
	CHECK_GRAPHIC_CONTEXT;

	ComPtr<IDXGIAdapter1> pAdapter;
	DXGraphic::EnumD3DAdapters(nullptr, [this, &pAdapter](void *userdata, ComPtr<IDXGIFactory1> factory, ComPtr<IDXGIAdapter1> adapter,
							      const DXGI_ADAPTER_DESC &desc, const char *version) {
		if (desc.AdapterLuid.HighPart == m_adapterLuid.HighPart && desc.AdapterLuid.LowPart == m_adapterLuid.LowPart) {
			m_pDX11Factory = factory;
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
				       (uint32_t)featureLevels.size(), D3D11_SDK_VERSION, m_pDX11Device.Assign(), &levelUsed, m_pDeviceContext.Assign());

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

	m_bBuildSuccessed = true;
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

void DX11GraphicInstanceImpl::SetRenderTarget(ComPtr<ID3D11RenderTargetView> target, uint32_t width, uint32_t height, ST_Color bkClr)
{
	CHECK_GRAPHIC_CONTEXT;

	ID3D11RenderTargetView *view = target.Get();
	m_pDeviceContext->OMSetRenderTargets(1, &view, NULL);

	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(vp));
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)0;
	vp.TopLeftY = (float)0;
	vp.Width = (float)width;
	vp.Height = (float)height;
	m_pDeviceContext->RSSetViewports(1, &vp);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_pDeviceContext->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);

	float color[4] = {bkClr.red, bkClr.green, bkClr.blue, bkClr.alpha};
	m_pDeviceContext->ClearRenderTargetView(target, color);

	m_pCurrentRenderTarget = target;
}

void DX11GraphicInstanceImpl::UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, void *data, size_t size)
{
	CHECK_GRAPHIC_CONTEXT;
	m_pDeviceContext->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
}

bool DX11GraphicInstanceImpl::GetResource(const std::vector<texture_handle> &textures, std::vector<ID3D11ShaderResourceView *> &resources)
{
	CHECK_GRAPHIC_CONTEXT;

	resources.clear();

	for (auto &item : textures) {
		auto tex = dynamic_cast<DX11Texture2D *>(item);
		assert(tex);
		if (!tex) {
			assert(false);
			return false;
		}

		if (TextureType::CanvasTarget == tex->m_textureInfo.usage) {
			assert(false);
			return false;
		}

		if (!tex->IsBuilt() || !tex->m_pTextureResView)
			return false;

		resources.push_back(tex->m_pTextureResView.Get());
	}

	return !resources.empty();
}

void DX11GraphicInstanceImpl::ApplyShader(DX11Shader *shader)
{
	CHECK_GRAPHIC_CONTEXT;

	uint32_t stride = shader->m_shaderInfo.perVertexSize;
	uint32_t offset = 0;
	ID3D11Buffer *buffer[1];

	buffer[0] = shader->m_pVertexBuffer;
	m_pDeviceContext->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
	m_pDeviceContext->IASetInputLayout(shader->m_pInputLayout);

	m_pDeviceContext->VSSetShader(shader->m_pVertexShader, NULL, 0);
	if (shader->m_pVSConstBuffer) {
		buffer[0] = shader->m_pVSConstBuffer;
		m_pDeviceContext->VSSetConstantBuffers(0, 1, buffer);
	}

	m_pDeviceContext->PSSetShader(shader->m_pPixelShader, NULL, 0);
	if (shader->m_pPSConstBuffer) {
		buffer[0] = shader->m_pPSConstBuffer;
		m_pDeviceContext->PSSetConstantBuffers(0, 1, buffer);
	}
}

bool DX11GraphicInstanceImpl::RenderBegin_Canvas(texture_handle hdl, ST_Color bkClr)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(!m_pCurrentRenderTarget && !m_pCurrentSwapChain);

	if (!m_bBuildSuccessed)
		return false;

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || obj->m_textureInfo.usage != TextureType::CanvasTarget) {
		assert(false);
		return false;
	}

	if (!obj->IsBuilt())
		return false;

	SetRenderTarget(obj->m_pRenderTargetView, obj->m_descTexture.Width, obj->m_descTexture.Height, bkClr);
	m_pCurrentSwapChain = nullptr;

	return true;
}

bool DX11GraphicInstanceImpl::RenderBegin_Display(display_handle hdl, ST_Color bkClr)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(!m_pCurrentRenderTarget && !m_pCurrentSwapChain);

	if (!m_bBuildSuccessed)
		return false;

	auto obj = dynamic_cast<DX11SwapChain *>(hdl);
	assert(obj);
	if (!obj)
		return false;

	obj->TestResizeSwapChain();
	if (!obj->IsBuilt())
		return false;

	if (!IsWindow(obj->m_hWnd))
		return false;

	SetRenderTarget(obj->m_pRenderTargetView, obj->m_dwWidth, obj->m_dwHeight, bkClr);
	m_pCurrentSwapChain = obj->m_pSwapChain;

	return true;
}

void DX11GraphicInstanceImpl::SetVertexBuffer(shader_handle hdl, void *buffer, size_t size)
{
	CHECK_GRAPHIC_CONTEXT;

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert((shader->m_shaderInfo.vertexCount * shader->m_shaderInfo.perVertexSize) == size);
		UpdateShaderBuffer(shader->m_pVertexBuffer, buffer, size);
	}
}

void DX11GraphicInstanceImpl::SetVSConstBuffer(shader_handle hdl, void *vsBuffer, size_t vsSize)
{
	CHECK_GRAPHIC_CONTEXT;

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert(shader->m_shaderInfo.vsBufferSize == vsSize);
		UpdateShaderBuffer(shader->m_pVSConstBuffer, vsBuffer, vsSize);
	}
}

void DX11GraphicInstanceImpl::SetPSConstBuffer(shader_handle hdl, void *psBuffer, size_t psSize)
{
	CHECK_GRAPHIC_CONTEXT;

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert(shader->m_shaderInfo.psBufferSize == psSize);
		UpdateShaderBuffer(shader->m_pPSConstBuffer, psBuffer, psSize);
	}
}

void DX11GraphicInstanceImpl::FillRectangle(shader_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (!shader || !shader->IsBuilt())
		return;

	ApplyShader(shader);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pDeviceContext->Draw(shader->m_shaderInfo.vertexCount, 0);
}

void DX11GraphicInstanceImpl::DrawTexture(shader_handle hdl, const std::vector<texture_handle> &textures)
{
	CHECK_GRAPHIC_CONTEXT;

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (!shader || !shader->IsBuilt())
		return;

	std::vector<ID3D11ShaderResourceView *> resources;
	if (!GetResource(textures, resources))
		return;

	ApplyShader(shader);

	ID3D11SamplerState *sampleState = m_pSampleState.Get();
	m_pDeviceContext->PSSetSamplers(0, 1, &sampleState);
	m_pDeviceContext->PSSetShaderResources(0, (uint32_t)resources.size(), resources.data());
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pDeviceContext->Draw(shader->m_shaderInfo.vertexCount, 0);
}

void DX11GraphicInstanceImpl::RenderEnd()
{
	CHECK_GRAPHIC_CONTEXT;

	if (m_pCurrentSwapChain)
		m_pCurrentSwapChain->Present(0, 0);

	m_pCurrentRenderTarget = nullptr;
	m_pCurrentSwapChain = nullptr;
}
