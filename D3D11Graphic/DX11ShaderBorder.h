#pragma once
#include "DX11Shader.h"

#define BORDER_VERTEX_COUNT 13

struct tBorderVertexType {
	float x, y, z, w;
};

class DX11GraphicInstanceImpl;
class DX11ShaderBorder : public DX11Shader {
public:
	DX11ShaderBorder(DX11GraphicInstanceImpl &graphic, const WCHAR *vsFile, const WCHAR *psFile);

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
};
