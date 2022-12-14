#include "DX11GraphicSession.h"

HMODULE DX11GraphicSession::s_hDllModule = nullptr;

#define CHECK_GRAPHIC_OBJECT_ALIVE(hdl, action)                                         \
	if (!IsGraphicObjectAlive(hdl)) {                                               \
		OutputDebugStringA("Using deleted object, cash must happen later! \n"); \
		assert(false);                                                          \
		action;                                                                 \
	}

DX11GraphicSession::DX11GraphicSession()
{
	InitializeCriticalSection(&m_lockOperation);
}

DX11GraphicSession::~DX11GraphicSession()
{
	assert(m_listObject.empty());
	DeleteCriticalSection(&m_lockOperation);
}

bool DX11GraphicSession::InitializeGraphic(const ST_GraphicCardInfo *graphic)
{
	CHECK_GRAPHIC_CONTEXT;

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(S_OK == hr);

	if (graphic)
		m_destGraphic = *graphic;
	else
		m_destGraphic = ST_GraphicCardInfo();

	return BuildAllDX();
}

void DX11GraphicSession::UnInitializeGraphic()
{
	CHECK_GRAPHIC_CONTEXT;

	ReleaseAllDX();

	if (!m_listObject.empty()) {
		assert(false && "you should destory all graphic objects");
		DestroyAllGraphicObject();
	}

	CoUninitialize();
}

void DX11GraphicSession::RegisterCallback(std::weak_ptr<DX11GraphicCallback> cb)
{
	CHECK_GRAPHIC_CONTEXT;
	m_pGraphicCallbacks.push_back(cb);
}

void DX11GraphicSession::UnRegisterCallback(DX11GraphicCallback *cb)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = m_pGraphicCallbacks.begin();
	while (itr != m_pGraphicCallbacks.end()) {
		auto temp = itr->lock();
		if (!temp || temp.get() == cb) {
			itr = m_pGraphicCallbacks.erase(itr);
			continue;
		}

		++itr;
	}
}

bool DX11GraphicSession::IsGraphicBuilt()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_bBuildSuccessed;
}

bool DX11GraphicSession::ReBuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT;
	return BuildAllDX();
}

void DX11GraphicSession::DestroyGraphicObject(DX11GraphicObject *&hdl)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!IsGraphicObjectAlive(hdl)) {
		OutputDebugStringA("You are trying to release a deleted object! \n");
		assert(false);
		return;
	}

	auto obj = dynamic_cast<DX11GraphicBase *>(hdl);
	assert(obj);
	if (obj)
		obj->ReleaseDX();

	delete hdl;
	hdl = nullptr;
}

void DX11GraphicSession::DestroyAllGraphicObject()
{
	CHECK_GRAPHIC_CONTEXT;

	auto temp = m_listObject;
	for (auto &item : temp) {
		OutputDebugStringA(item->GetName());

		item->ReleaseDX();
		delete item;
	}

	m_listObject.clear();
}

texture_handle DX11GraphicSession::OpenSharedTexture(HANDLE hSharedHanle)
{
	CHECK_GRAPHIC_CONTEXT;

	DX11Texture2D *tex = new DX11Texture2D(*this, hSharedHanle);
	if (!tex->IsBuilt()) {
		delete tex;
		return nullptr;
	}

	return tex;
}

texture_handle DX11GraphicSession::OpenImageTexture(const WCHAR *fullPath)
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

texture_handle DX11GraphicSession::CreateTexture(const ST_TextureInfo &info)
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

ST_TextureInfo DX11GraphicSession::GetTextureInfo(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return ST_TextureInfo());

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return ST_TextureInfo();

	return ST_TextureInfo(obj->m_descTexture.Width, obj->m_descTexture.Height,
			      obj->m_descTexture.Format, obj->m_textureInfo.usage);
}

HANDLE DX11GraphicSession::GetSharedHandle(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return 0);

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return 0;

	return obj->m_hSharedHandle;
}

bool DX11GraphicSession::CopyDisplay(texture_handle dest, display_handle src)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(dest, return false);
	CHECK_GRAPHIC_OBJECT_ALIVE(src, return false);

	if (!m_bBuildSuccessed)
		return false;

	auto destTex = dynamic_cast<DX11Texture2D *>(dest);
	assert(destTex);
	if (!destTex || !destTex->IsBuilt())
		return false;

	auto srcDisplay = dynamic_cast<DX11SwapChain *>(src);
	assert(srcDisplay);
	if (!srcDisplay || !srcDisplay->IsBuilt())
		return false;

	if (!IsTextureInfoSame(&destTex->m_descTexture, &srcDisplay->m_descTexture)) {
		assert(false);
		return false;
	}

	m_pDeviceContext->CopyResource(destTex->m_pTexture2D, srcDisplay->m_pSwapBackTexture2D);
	return true;
}

bool DX11GraphicSession::CopyTexture(texture_handle dest, texture_handle src)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(dest, return false);
	CHECK_GRAPHIC_OBJECT_ALIVE(src, return false);

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

	if (!IsTextureInfoSame(&destTex->m_descTexture, &srcTex->m_descTexture)) {
		assert(false);
		return false;
	}

	m_pDeviceContext->CopyResource(destTex->m_pTexture2D, srcTex->m_pTexture2D);
	return true;
}

bool DX11GraphicSession::MapTexture(texture_handle hdl, MapTextureType type,
				    D3D11_MAPPED_SUBRESOURCE *mapData)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return false);

	if (!m_bBuildSuccessed)
		return false;

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return false;

	D3D11_MAP method = (type == MapTextureType::MapRead) ? D3D11_MAP_READ
							     : D3D11_MAP_WRITE_DISCARD;
	HRESULT hr = m_pDeviceContext->Map(obj->m_pTexture2D, 0, method, 0, mapData);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	return true;
}

void DX11GraphicSession::UnmapTexture(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || !obj->IsBuilt()) {
		assert(false);
		return;
	}

	m_pDeviceContext->Unmap(obj->m_pTexture2D, 0);
}

display_handle DX11GraphicSession::CreateDisplay(HWND hWnd)
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

void DX11GraphicSession::SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto obj = dynamic_cast<DX11SwapChain *>(hdl);
	assert(obj);
	if (obj)
		obj->SetDisplaySize(width, height);
}

ST_DisplayInfo DX11GraphicSession::GetDisplayInfo(display_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return ST_DisplayInfo());

	auto obj = dynamic_cast<DX11SwapChain *>(hdl);
	assert(obj);
	if (!obj || !obj->IsBuilt())
		return ST_DisplayInfo();

	return ST_DisplayInfo(obj->m_hWnd, obj->m_descTexture.Width, obj->m_descTexture.Height,
			      obj->m_descTexture.Format);
}

shader_handle DX11GraphicSession::CreateShader(const ST_ShaderInfo &info)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(!info.vertexDesc.empty());
	auto ret = new DX11Shader(*this, &info);
	if (!ret->IsBuilt()) {
		delete ret;
		assert(false);
		return nullptr;
	}

	return ret;
}

ComPtr<IDXGIFactory1> DX11GraphicSession::DXFactory()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Factory;
}

ComPtr<ID3D11Device> DX11GraphicSession::DXDevice()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Device;
}

ComPtr<ID3D11DeviceContext> DX11GraphicSession::DXContext()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDeviceContext;
}

void DX11GraphicSession::PushObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	m_listObject.push_back(obj);
}

void DX11GraphicSession::RemoveObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = find(m_listObject.begin(), m_listObject.end(), obj);
	if (itr != m_listObject.end())
		m_listObject.erase(itr);
}

void DX11GraphicSession::ReleaseAllDX()
{
	CHECK_GRAPHIC_CONTEXT;

	m_bBuildSuccessed = false;

	for (auto &item : m_listObject)
		item->ReleaseDX();

	m_pDX11Factory = nullptr;
	m_pDX11Device = nullptr;
	m_pDeviceContext = nullptr;
	m_pBlendState = nullptr;
	m_pSampleStateAnisotropic = m_pSampleStatePoint = m_pSampleStateLinear = nullptr;
}

bool DX11GraphicSession::BuildAllDX()
{
	CHECK_GRAPHIC_CONTEXT;

	std::optional<GraphicCardType> currentType;
	DXGraphic::EnumD3DAdapters(nullptr, [this, &currentType](void *userdata,
								 ComPtr<IDXGIFactory1> factory,
								 ComPtr<IDXGIAdapter1> adapter,
								 const DXGI_ADAPTER_DESC &desc,
								 const char *version) {
		if (m_destGraphic.vendorId || m_destGraphic.deviceId) {
			if (desc.VendorId == m_destGraphic.vendorId &&
			    desc.DeviceId == m_destGraphic.deviceId) {
				m_pDX11Factory = factory;
				m_pAdapter = adapter;
				return false;
			}
		} else {
			GraphicCardType newType = DXGraphic::CheckAdapterType(desc);
			if (!currentType.has_value() ||
			    g_mapGraphicOrder[newType] < g_mapGraphicOrder[currentType.value()]) {
				currentType = newType;
				m_pDX11Factory = factory;
				m_pAdapter = adapter;
			}
		}

		return true;
	});

	if (!m_pAdapter)
		return false;

	DXGI_ADAPTER_DESC descAdapter;
	m_pAdapter->GetDesc(&descAdapter);

#ifdef _DEBUG
	OutputDebugStringW(L"Select default device: ");
	OutputDebugStringW(descAdapter.Description);
	OutputDebugStringW(L"\n");
#endif

	D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_10_0;
	HRESULT hr = D3D11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
				       D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels.data(),
				       (uint32_t)featureLevels.size(), D3D11_SDK_VERSION,
				       m_pDX11Device.Assign(), &levelUsed,
				       m_pDeviceContext.Assign());

	if (FAILED(hr)) {
		CheckDXError(hr);
		return false;
	}

	OutputDebugStringA("DX11Device is using level with :");

	if (!InitBlendState())
		return false;

	if (!InitSamplerState())
		return false;

	for (auto &item : m_listObject)
		item->BuildDX();

	for (auto &item : m_pGraphicCallbacks) {
		auto cb = item.lock();
		if (cb)
			cb->OnBuildSuccessed(descAdapter);
	}

	m_bBuildSuccessed = true;
	return true;
}

bool DX11GraphicSession::InitBlendState()
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

	HRESULT hr =
		m_pDX11Device->CreateBlendState(&blendStateDescription, m_pBlendState.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	return true;
}

bool DX11GraphicSession::InitSamplerState()
{
	CHECK_GRAPHIC_CONTEXT;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = FLT_MAX;

	//------------------------------------------------------------------------------------------
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	HRESULT hr =
		m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStateAnisotropic.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = 0;
	hr = m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStateLinear.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = 0;
	hr = m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStatePoint.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	return true;
}

void DX11GraphicSession::SetRenderTarget(ComPtr<ID3D11RenderTargetView> target, uint32_t width,
					 uint32_t height)
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

	m_pCurrentRenderTarget = target;
}

void DX11GraphicSession::UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, const void *data,
					    size_t size)
{
	CHECK_GRAPHIC_CONTEXT;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	assert(desc.ByteWidth == size);
	if (desc.ByteWidth == size)
		m_pDeviceContext->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
}

bool DX11GraphicSession::GetResource(const std::vector<texture_handle> &textures,
				     std::vector<ID3D11ShaderResourceView *> &resources)
{
	CHECK_GRAPHIC_CONTEXT;

	resources.clear();

	for (auto &item : textures) {
		CHECK_GRAPHIC_OBJECT_ALIVE(item, continue);

		auto tex = dynamic_cast<DX11Texture2D *>(item);
		assert(tex);
		if (!tex) {
			assert(false);
			return false;
		}

		if (!tex->IsBuilt() || !tex->m_pTextureResView)
			return false;

		resources.push_back(tex->m_pTextureResView.Get());
	}

	return !resources.empty();
}

void DX11GraphicSession::ApplyShader(DX11Shader *shader)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(shader, return );

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

bool DX11GraphicSession::BeginRenderCanvas(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return false);
	assert(!m_pCurrentRenderTarget && !m_pCurrentSwapChain);

	if (!m_bBuildSuccessed) {
		if (!BuildAllDX())
			return false;
	}

	auto obj = dynamic_cast<DX11Texture2D *>(hdl);
	assert(obj);
	if (!obj || obj->m_textureInfo.usage != TextureType::CanvasTarget) {
		assert(false);
		return false;
	}

	if (!obj->IsBuilt())
		return false;

	SetRenderTarget(obj->m_pRenderTargetView, obj->m_descTexture.Width,
			obj->m_descTexture.Height);
	m_pCurrentSwapChain = nullptr;

	EnterContext(std::source_location::current());
	return true;
}

bool DX11GraphicSession::BeginRenderWindow(display_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return false);
	assert(!m_pCurrentRenderTarget && !m_pCurrentSwapChain);

	if (!m_bBuildSuccessed) {
		if (!BuildAllDX())
			return false;
	}

	auto obj = dynamic_cast<DX11SwapChain *>(hdl);
	assert(obj);
	if (!obj)
		return false;

	if (!IsWindow(obj->m_hWnd))
		return false;

	HRESULT hr = obj->TestResizeSwapChain();
	if (FAILED(hr)) {
		CheckDXError(hr);
		HandleDXHResult(hr);
		return false;
	}

	SetRenderTarget(obj->m_pRenderTargetView, obj->m_dwWidth, obj->m_dwHeight);
	m_pCurrentSwapChain = obj->m_pSwapChain;

	EnterContext(std::source_location::current());
	return true;
}

void DX11GraphicSession::ClearBackground(const ST_Color *bkClr)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_pDeviceContext || !m_pCurrentRenderTarget) {
		assert(false);
		return;
	}

	float color[4] = {bkClr->red, bkClr->green, bkClr->blue, bkClr->alpha};
	m_pDeviceContext->ClearRenderTargetView(m_pCurrentRenderTarget, color);
}

void DX11GraphicSession::SetBlendState(BlendStateType type)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_pDeviceContext || !m_pCurrentRenderTarget) {
		assert(false);
		return;
	}

	switch (type) {
	case BlendStateType::Normal:
		if (m_pBlendState) {
			float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
			m_pDeviceContext->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);
		}
		break;

	case BlendStateType::Disable:
	default:
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		break;
	}
}

void DX11GraphicSession::SetVertexBuffer(shader_handle hdl, const void *buffer, size_t size)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert((shader->m_shaderInfo.vertexCount * shader->m_shaderInfo.perVertexSize) ==
		       size);
		UpdateShaderBuffer(shader->m_pVertexBuffer, buffer, size);
	}
}

void DX11GraphicSession::SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert(shader->m_shaderInfo.vsBufferSize == vsSize);
		UpdateShaderBuffer(shader->m_pVSConstBuffer, vsBuffer, vsSize);
	}
}

void DX11GraphicSession::SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (shader && shader->IsBuilt()) {
		assert(shader->m_shaderInfo.psBufferSize == psSize);
		UpdateShaderBuffer(shader->m_pPSConstBuffer, psBuffer, psSize);
	}
}

void DX11GraphicSession::DrawTopplogy(shader_handle hdl, D3D11_PRIMITIVE_TOPOLOGY type)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (!shader || !shader->IsBuilt())
		return;

	ApplyShader(shader);

	m_pDeviceContext->IASetPrimitiveTopology(type);
	m_pDeviceContext->Draw(shader->m_shaderInfo.vertexCount, 0);
}

void DX11GraphicSession::DrawTexture(shader_handle hdl, FilterType flt,
				     const std::vector<texture_handle> &textures)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_ALIVE(hdl, return );

	auto shader = dynamic_cast<DX11Shader *>(hdl);
	assert(shader);
	if (!shader || !shader->IsBuilt())
		return;

	std::vector<ID3D11ShaderResourceView *> resources;
	if (!GetResource(textures, resources))
		return;

	ApplyShader(shader);

	ID3D11SamplerState *sampleState = nullptr;
	switch (flt) {
	case FilterType::FilterPoint:
		sampleState = m_pSampleStatePoint.Get();
		break;

	case FilterType::FilterAnisotropic:
		sampleState = m_pSampleStateAnisotropic.Get();
		break;

	case FilterType::FilterLinear:
	default:
		sampleState = m_pSampleStateLinear.Get();
		break;
	}

	if (sampleState)
		m_pDeviceContext->PSSetSamplers(0, 1, &sampleState);

	m_pDeviceContext->PSSetShaderResources(0, (uint32_t)resources.size(), resources.data());
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pDeviceContext->Draw(shader->m_shaderInfo.vertexCount, 0);
}

void DX11GraphicSession::EndRender()
{
	CHECK_GRAPHIC_CONTEXT;

	HRESULT hr = S_OK;
	if (m_pCurrentSwapChain) {
		hr = m_pCurrentSwapChain->Present(0, 0);
	}

	m_pCurrentRenderTarget = nullptr;
	m_pCurrentSwapChain = nullptr;

	HandleDXHResult(hr);

	LeaveContext(std::source_location::current());
}

void DX11GraphicSession::HandleDXHResult(HRESULT hr, std::source_location location)
{
	CHECK_GRAPHIC_CONTEXT;

	if (FAILED(hr)) {
		for (auto &item : m_pGraphicCallbacks) {
			auto cb = item.lock();
			if (cb)
				cb->OnD3D11Error(hr);
		}

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
			for (auto &item : m_pGraphicCallbacks) {
				auto cb = item.lock();
				if (cb)
					cb->OnDeviceRemoved();
			}

			ReleaseAllDX();
			BuildAllDX();
		}
	}
}

bool DX11GraphicSession::IsGraphicObjectAlive(DX11GraphicObject *obj)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = find(m_listObject.begin(), m_listObject.end(), obj);
	if (itr != m_listObject.end())
		return true;

	return false;
}

bool DX11GraphicSession::IsTextureInfoSame(const D3D11_TEXTURE2D_DESC *desc,
					   const D3D11_TEXTURE2D_DESC *src)
{
	return ((desc->Width == src->Width) && (desc->Height == src->Height) &&
		(desc->Format == src->Format));
}
