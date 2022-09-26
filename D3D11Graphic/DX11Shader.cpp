#include "DX11Shader.h"
#include <d3dcompiler.h>

std::wstring GetExeDir()
{
	WCHAR szFilePath[MAX_PATH] = {};
	GetModuleFileNameW(NULL, szFilePath, MAX_PATH);

	int nLen = (int)wcslen(szFilePath);
	for (int i = nLen - 1; i >= 0; --i) {
		if (szFilePath[i] == '\\') {
			szFilePath[i + 1] = 0;
			break;
		}
	}

	return std::wstring(szFilePath);
}

bool DX11Shader::InitShader(ComPtr<ID3D11Device> pDevice, const WCHAR *vsFile, const WCHAR *psFile, int vertexSize, int vsBufferSize,
			    int psBufferSize)
{
	std::wstring dir = GetExeDir();
	std::wstring vs = dir + vsFile;
	std::wstring ps = dir + psFile;

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

	hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
					 m_pVertexShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
					m_pPixelShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[2] = {};
	inputLayoutDesc[0].SemanticName = "SV_POSITION";
	inputLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	inputLayoutDesc[1].SemanticName = "TEXCOORD";
	inputLayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDesc[1].AlignedByteOffset = 16;

	unsigned int numElements = sizeof(inputLayoutDesc) / sizeof(inputLayoutDesc[0]);
	hr = pDevice->CreateInputLayout(inputLayoutDesc, numElements, vertexShaderBuffer->GetBufferPointer(),
					vertexShaderBuffer->GetBufferSize(), m_pInputLayout.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = vertexSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = pDevice->CreateBuffer(&vertexBufferDesc, NULL, m_pVertexBuffer.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (vsBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = vsBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = pDevice->CreateBuffer(&CBufferDesc, NULL, m_pVSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	if (psBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = psBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = pDevice->CreateBuffer(&CBufferDesc, NULL, m_pPSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	return true;
}
