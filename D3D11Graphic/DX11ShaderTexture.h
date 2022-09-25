#pragma once
#include "DX11Shader.h"

class DX11GraphicInstanceImpl;
class DX11ShaderTexture : public DX11Shader {
public:
	DX11ShaderTexture(DX11GraphicInstanceImpl &graphic, const ST_ShaderInfo *info);

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
};
