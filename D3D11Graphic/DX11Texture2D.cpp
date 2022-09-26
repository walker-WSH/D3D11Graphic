#include "DX11Texture2D.h"
#include "DX11GraphicInstanceImpl.h"
#include <d3dcompiler.h>

DX11Texture2D::DX11Texture2D(DX11GraphicInstanceImpl &graphic, uint32_t width, uint32_t height, enum DXGI_FORMAT format, TextureType type)
	: DX11Object(graphic), m_dwWidth(width), m_dwHeight(height), m_format(format), m_usage(type)
{
	BuildDX();
}

DX11Texture2D::DX11Texture2D(DX11GraphicInstanceImpl &graphic, HANDLE handle) : DX11Object(graphic), m_hSharedHandle(handle), m_usage(TextureType::SharedHandle)
{
	BuildDX();
}

bool DX11Texture2D::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	bool bSuccessed = false;
	switch (m_usage) {
	case TextureType::RenderTarget:
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

	m_pTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
	m_pTextureResView = nullptr;
}

bool DX11Texture2D::InitWriteTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_dwWidth;
	desc.Height = m_dwHeight;
	desc.Format = m_format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	D3D11_MAPPED_SUBRESOURCE map;
	hr = m_graphic.DXContext()->Map(m_pTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	m_graphic.DXContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitReadTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_dwWidth;
	desc.Height = m_dwHeight;
	desc.Format = m_format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE map;
	hr = m_graphic.DXContext()->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &map);
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	m_graphic.DXContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitTargetTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_dwWidth;
	desc.Height = m_dwHeight;
	desc.Format = m_format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE | D3D11_RESOURCE_MISC_SHARED;

	HRESULT hr = m_graphic.DXDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreateRenderTargetView(m_pTexture2D, nullptr, m_pRenderTargetView.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	return true;
}

bool DX11Texture2D::InitSharedTexture()
{
	HRESULT hr = m_graphic.DXDevice()->OpenSharedResource((HANDLE)m_hSharedHandle, __uuidof(ID3D11Texture2D), (void **)m_pTexture2D.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

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

	HRESULT hr = m_graphic.DXDevice()->CreateShaderResourceView(m_pTexture2D, &viewDesc, m_pTextureResView.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}
