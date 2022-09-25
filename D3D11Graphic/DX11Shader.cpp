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

DX11Shader::DX11Shader(DX11GraphicInstanceImpl &graphic, const ST_ShaderInfo *info) : DX11GraphicBase(graphic), m_shaderInfo(*info) {}

bool DX11Shader::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	std::wstring dir = GetHLSLDir();
	std::wstring vs = dir + m_shaderInfo.vsFile;
	std::wstring ps = dir + m_shaderInfo.psFile;

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;

	HRESULT hr = D3DReadFileToBlob(vs.c_str(), vertexShaderBuffer.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = D3DReadFileToBlob(ps.c_str(), pixelShaderBuffer.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
						      m_pVertexShader.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	hr = m_graphic.DXDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = GetInputLayout();
	hr = m_graphic.DXDevice()->CreateInputLayout(inputLayoutDesc.data(), (uint32_t)inputLayoutDesc.size(), vertexShaderBuffer->GetBufferPointer(),
						     vertexShaderBuffer->GetBufferSize(), m_pInputLayout.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = m_shaderInfo.vertexCount * m_shaderInfo.perVertexSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = m_graphic.DXDevice()->CreateBuffer(&vertexBufferDesc, NULL, m_pVertexBuffer.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return false;
	}

	if (m_shaderInfo.vsBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_shaderInfo.vsBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.DXDevice()->CreateBuffer(&CBufferDesc, NULL, m_pVSConstBuffer.Assign());
		if (FAILED(hr)) {
			CheckDXError(hr);
			assert(false);
			return false;
		}
	}

	if (m_shaderInfo.psBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_shaderInfo.psBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.DXDevice()->CreateBuffer(&CBufferDesc, NULL, m_pPSConstBuffer.Assign());
		if (FAILED(hr)) {
			CheckDXError(hr);
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
	m_pVSConstBuffer = nullptr;

	m_pPixelShader = nullptr;
	m_pPSConstBuffer = nullptr;

	m_pInputLayout = nullptr;
	m_pVertexBuffer = nullptr;
}
