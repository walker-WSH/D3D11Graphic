#include "DX11ShaderTexture.h"

DX11ShaderTexture::DX11ShaderTexture(DX11GraphicInstanceImpl &graphic, const ST_ShaderInfo *info)
	: DX11Shader(graphic, info)
{
	BuildDX();
}

std::vector<D3D11_INPUT_ELEMENT_DESC> DX11ShaderTexture::GetInputLayout()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	D3D11_INPUT_ELEMENT_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_INPUT_ELEMENT_DESC));
	desc.SemanticName = "SV_POSITION";
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDesc.push_back(desc);

	ZeroMemory(&desc, sizeof(D3D11_INPUT_ELEMENT_DESC));
	desc.SemanticName = "TEXCOORD";
	desc.Format = DXGI_FORMAT_R32G32_FLOAT;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc.AlignedByteOffset = 16;
	inputLayoutDesc.push_back(desc);

	return inputLayoutDesc;
}
