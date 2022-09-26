#pragma once
#include "DX11Shader.h"

struct tBorderVertexType {
	float x, y, z, w;
};

class DX11GraphicInstanceImpl;
class DX11ShaderBorder : public DX11Shader {
public:
	DX11ShaderBorder(DX11GraphicInstanceImpl &graphic, const ST_ShaderInfo *info);

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
};
