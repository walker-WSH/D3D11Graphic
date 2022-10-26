#pragma once
#include <DX11GraphicBase.h>

class DX11GraphicInstanceImpl;
class DX11Shader : public DX11GraphicBase {
	friend class DX11GraphicInstanceImpl;

public:
	DX11Shader(DX11GraphicInstanceImpl &graphic, const ST_ShaderInfo *info);

	virtual const char *GetName() { return "shader"; }
	virtual bool BuildDX();
	virtual void ReleaseDX();
	virtual bool IsBuilt()
	{
		return m_pVertexShader && m_pPixelShader && m_pInputLayout && m_pVertexBuffer;
	}

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout() = 0;

protected:
	const ST_ShaderInfo m_shaderInfo;

	// vertex shader
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;
	ComPtr<ID3D11Buffer> m_pVSConstBuffer = nullptr;

	// pixel shader
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;
	ComPtr<ID3D11Buffer> m_pPSConstBuffer = nullptr;

	// vertex info
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;
};
