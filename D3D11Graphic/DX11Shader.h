#pragma once
#include <Windows.h>
#include <string>
#include <DXDefine.h>

struct DX11Shader {
	// vertex shader
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11Buffer> m_pVSBuffer;

	// pixel shader
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11Buffer> m_pPSBuffer;

	// vertex info
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;

public:
	bool InitShader(ComPtr<ID3D11Device> pDevice, const WCHAR *vsFile, const WCHAR *psFile, int vertexSize, int vsBufferSize,
			int psBufferSize);
};
