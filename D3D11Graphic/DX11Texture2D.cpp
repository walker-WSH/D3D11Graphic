#include "DX11Texture2D.h"
#include "DX11GraphicSession.h"
#include <d3dcompiler.h>
#include <dxsdk/include/D3DX11tex.h>

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, const ST_TextureInfo &info)
	: DX11GraphicBase(graphic), m_textureInfo(info)
{
	BuildDX();
}

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, HANDLE handle)
	: DX11GraphicBase(graphic), m_hSharedHandle(handle)
{
	m_textureInfo.usage = TextureType::SharedHandle;
	BuildDX();
}

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, const WCHAR *fullPath)
	: DX11GraphicBase(graphic), m_strImagePath(fullPath)
{
	m_textureInfo.usage = TextureType::StaticImageFile;
	BuildDX();
}

bool DX11Texture2D::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	bool bSuccessed = false;
	switch (m_textureInfo.usage) {
	case TextureType::CanvasTarget:
		bSuccessed = InitTargetTexture();
		break;

	case TextureType::ReadTexture:
		bSuccessed = InitReadTexture();
		break;

	case TextureType::WriteTexture:
		bSuccessed = InitWriteTexture();
		break;

	case TextureType::SharedHandle:
		bSuccessed = InitSharedTexture();
		break;

	case TextureType::StaticImageFile:
		bSuccessed = InitImageTexture();
		break;

	default:
		assert(false);
		break;
	}

	if (bSuccessed)
		m_pTexture2D->GetDesc(&m_descTexture);
	else
		ReleaseDX();

	return bSuccessed;
}

void DX11Texture2D::ReleaseDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	ZeroMemory(&m_descTexture, sizeof(D3D11_TEXTURE2D_DESC));
	m_pTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
	m_pTextureResView = nullptr;
}

bool DX11Texture2D::InitWriteTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	D3D11_MAPPED_SUBRESOURCE map;
	hr = m_graphic.DXContext()->Map(m_pTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	m_graphic.DXContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitReadTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE map;
	hr = m_graphic.DXContext()->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &map);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	m_graphic.DXContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitTargetTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
	if (DXGI_FORMAT_B8G8R8A8_UNORM == m_textureInfo.format)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreateRenderTargetView(m_pTexture2D, nullptr,
							  m_pRenderTargetView.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	ComPtr<IDXGIResource> pDXGIRes = nullptr;
	hr = m_pTexture2D->QueryInterface(__uuidof(IDXGIResource), (LPVOID *)(pDXGIRes.Assign()));
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = pDXGIRes->GetSharedHandle(&m_hSharedHandle);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	if (m_textureInfo.format == D2D_TEXTURE_FORMAT)
		testD2D();

	return true;
}

void DX11Texture2D::testD2D()
{
	ComPtr<IDXGISurface1> sfc;
	auto hr = m_graphic.DXDevice()->OpenSharedResource(m_hSharedHandle, __uuidof(IDXGISurface1),
							   (LPVOID *)(sfc.Assign()));
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return;
	}

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pD2D1Factory);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return;
	}

	auto prp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
						D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,
								  D2D1_ALPHA_MODE_PREMULTIPLIED));
	hr = m_pD2D1Factory->CreateDxgiSurfaceRenderTarget(sfc.Get(), &prp, &m_pD2D1RenderTarget);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return;
	}

	m_pD2D1RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_pD2D1RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	hr = m_pD2D1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 1.f, 1.f),
							m_pD2D1SolidBrush.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return;
	}

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(
		D2D1::Point2F((float)m_textureInfo.width / 2, (float)m_textureInfo.height / 2),
		(float)m_textureInfo.width / 3, (float)m_textureInfo.height / 3);

	m_pD2D1RenderTarget->BeginDraw();
	m_pD2D1RenderTarget->Clear(D2D1::ColorF(1.f, 0, 0, 0.5f));
	m_pD2D1RenderTarget->FillEllipse(ellipse, m_pD2D1SolidBrush);
	hr = m_pD2D1RenderTarget->EndDraw();

	if (D2DERR_RECREATE_TARGET == hr) {
		// device error and reinitialize
	} else if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return;
	}
}

bool DX11Texture2D::InitSharedTexture()
{
	HRESULT hr = m_graphic.DXDevice()->OpenSharedResource(
		(HANDLE)m_hSharedHandle, __uuidof(ID3D11Texture2D), (void **)m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		return false;
	}

	if (!InitResourceView())
		return false;

	return true;
}

bool DX11Texture2D::InitImageTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_graphic.DXDevice(),
							    m_strImagePath.c_str(), NULL, NULL,
							    m_pTextureResView.Assign(), NULL);
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	ComPtr<ID3D11Resource> pResource;
	m_pTextureResView->GetResource(pResource.Assign());
	if (!pResource) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = pResource->QueryInterface<ID3D11Texture2D>(m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	return true;
}

bool DX11Texture2D::InitResourceView()
{
	D3D11_TEXTURE2D_DESC desc = {};
	m_pTexture2D->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = desc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;

	HRESULT hr = m_graphic.DXDevice()->CreateShaderResourceView(m_pTexture2D, &viewDesc,
								    m_pTextureResView.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	return true;
}
