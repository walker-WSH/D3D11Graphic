#pragma once
#include <DX11GraphicBase.h>

class DX11GraphicSession;
class DX11Shader : public DX11GraphicBase {
	friend class DX11GraphicSession;

public:
	DX11Shader(DX11GraphicSession &graphic, const ST_ShaderInfo *info);

	virtual const char *GetName() { return "shader"; }
	virtual bool BuildDX();
	virtual void ReleaseDX();
	virtual bool IsBuilt();

protected:
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
	void GetSemanticName(VertexInputType type, D3D11_INPUT_ELEMENT_DESC &desc);
	void GetSemanticFormat(uint32_t size, D3D11_INPUT_ELEMENT_DESC &desc);

private:
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
