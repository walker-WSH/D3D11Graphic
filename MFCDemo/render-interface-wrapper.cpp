#include "pch.h"
#include "render-interface-wrapper.h"

IDX11GraphicInstance *pGraphic = nullptr;
std::map<ShaderType, shader_handle> shaders;

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	shader_handle shader = shaders[ShaderType::shaderTexture];
	ST_TextureInfo texInfo = pGraphic->GetTextureInfo(texs.at(0));
	SIZE texSize(texInfo.width, texInfo.height);
	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));

	pGraphic->DrawTexture(shader, texs);
}

void FillRectangle(SIZE canvas, RECT drawDest, ST_Color clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderFillRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], &clr, sizeof(ST_Color));

	pGraphic->DrawTopplogy(shaders[type], D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void RenderBorderWithSize(SIZE canvas, RECT drawDest, long borderSize, ST_Color clr)
{
	RECT rcLeft;
	RECT rcRight;
	RECT rcTop;
	RECT rcBottom;

	rcLeft.right = drawDest.left;
	rcLeft.left = drawDest.left - borderSize;
	rcLeft.top = drawDest.top - borderSize;
	rcLeft.bottom = drawDest.bottom + borderSize;

	rcRight.left = drawDest.right;
	rcRight.right = drawDest.right + borderSize;
	rcRight.top = drawDest.top - borderSize;
	rcRight.bottom = drawDest.bottom + borderSize;

	rcTop.left = drawDest.left - borderSize;
	rcTop.right = drawDest.right + borderSize;
	rcTop.top = drawDest.top - borderSize;
	rcTop.bottom = drawDest.top;

	rcBottom.left = drawDest.left - borderSize;
	rcBottom.right = drawDest.right + borderSize;
	rcBottom.top = drawDest.bottom;
	rcBottom.bottom = drawDest.bottom + borderSize;

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	FillRectangle(canvas, rcLeft, clr);
	FillRectangle(canvas, rcRight, clr);
	FillRectangle(canvas, rcTop, clr);
	FillRectangle(canvas, rcBottom, clr);
}
