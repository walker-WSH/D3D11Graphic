#pragma once
#include "DX11Shader.h"

#define TEXTURE_VERTEX_COUNT 4

struct tTextureVertexType {
	float x, y, z, w;
	float u, v;
};

class DX11GraphicInstanceImpl;
class DX11ShaderTexture : public DX11Shader {
public:
	DX11ShaderTexture(DX11GraphicInstanceImpl &graphic, const WCHAR *vsFile, const WCHAR *psFile, int vsBufferSize, int psBufferSize);

protected:
	virtual std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
};
