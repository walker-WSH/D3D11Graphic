#include "DX11Shader.h"
#include "DX11GraphicInstanceImpl.h"
#include <d3dcompiler.h>

std::wstring GetHLSLDir()
{
	static std::wstring s_strHLSLDir = L"";

	if (s_strHLSLDir.empty()) {
		WCHAR szFilePath[MAX_PATH] = {};
		GetModuleFileNameW(DX11GraphicInstanceImpl::s_hDllModule, szFilePath, MAX_PATH);

		int nLen = (int)wcslen(szFilePath);
		for (int i = nLen - 1; i >= 0; --i) {
			if (szFilePath[i] == '\\') {
				szFilePath[i + 1] = 0;
				break;
			}
		}

		s_strHLSLDir = std::wstring(szFilePath) + L"HLSL\\";
	}

	return s_strHLSLDir;
}

DX11Shader::DX11Shader(DX11GraphicInstanceImpl &graphic, const WCHAR *vsFile, const WCHAR *psFile, int vertexSize, int vsBufferSize,
		       int psBufferSize)
	: DX11Object(graphic),
	  m_strVSFile(vsFile),
	  m_strPSFile(psFile),
	  m_nVertexSize(vertexSize),
	  m_nVSBufferSize(vsBufferSize),
	  m_nPSBufferSize(psBufferSize)
{
}

bool DX11Shader::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	std::wstring dir = GetHLSLDir();
	std::wstring vs = dir + m_strVSFile;
	std::wstring ps = dir + m_strPSFile;

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;

	HRESULT hr = D3DReadFileToBlob(vs.c_str(), vertexShaderBuffer.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = D3DReadFileToBlob(ps.c_str(), pixelShaderBuffer.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
						      m_pVertexShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
						     m_pPixelShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = GetInputLayout();
	hr = m_graphic.DXDevice()->CreateInputLayout(inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(),
						     vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
						     m_pInputLayout.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = m_nVertexSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = m_graphic.DXDevice()->CreateBuffer(&vertexBufferDesc, NULL, m_pVertexBuffer.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (m_nVSBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_nVSBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.DXDevice()->CreateBuffer(&CBufferDesc, NULL, m_pVSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	if (m_nPSBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_nPSBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.DXDevice()->CreateBuffer(&CBufferDesc, NULL, m_pPSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	return true;
}

void DX11Shader::ReleaseDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	m_pVertexShader = nullptr;
	m_pVSBuffer = nullptr;

	m_pPixelShader = nullptr;
	m_pPSBuffer = nullptr;

	m_pInputLayout = nullptr;
	m_pVertexBuffer = nullptr;
}
