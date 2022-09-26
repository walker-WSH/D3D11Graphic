#pragma once
#include <DX11Object.h>

class DX11GraphicInstanceImpl;
class DX11Shader : public DX11Object {
public:
	DX11Shader(DX11GraphicInstanceImpl &graphic, const WCHAR *vsFile, const WCHAR *psFile, int vertexSize, int vsBufferSize,
		   int psBufferSize);

	virtual bool BuildDX();
	virtual void ReleaseDX();

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout() = 0;

protected:
	const std::wstring m_strVSFile;
	const std::wstring m_strPSFile;
	const int m_nVertexSize = 0;
	const int m_nVSBufferSize = 0;
	const int m_nPSBufferSize = 0;

	// vertex shader
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11Buffer> m_pVSBuffer;

	// pixel shader
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11Buffer> m_pPSBuffer;

	// vertex info
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
};
