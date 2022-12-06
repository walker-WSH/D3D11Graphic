#include "pch.h"
#include "render-interface-wrapper.h"

IDX11GraphicInstance *pGraphic = nullptr;
std::map<ShaderType, shader_handle> shaders;

void InitShader()
{
	float matrixWVP[4][4];
	ST_ShaderInfo shaderInfo;

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"default-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = 0;
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::shaderTexture] = shader;
	}

	{
		shaderInfo.vsFile = L"fill-rect-vs.cso";
		shaderInfo.psFile = L"fill-rect-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(ST_Color);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::shaderFillRect] = shader;
	}

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"rgb-to-y-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(toyuv_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::yuvOnePlane] = shader;
	}

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"rgb-to-uv-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(toyuv_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::uvPlane] = shader;
	}

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"i420-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::i420ToRGB] = shader;
	}

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"nv12-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::nv12ToRGB] = shader;
	}

	{
		shaderInfo.vsFile = L"default-vs.cso";
		shaderInfo.psFile = L"yuy2-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::yuy2ToRGB] = shader;
	}
}

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	shader_handle shader = shaders[ShaderType::shaderTexture];
	ST_TextureInfo texInfo = pGraphic->GetTextureInfo(texs.at(0));
	SIZE texSize(texInfo.width, texInfo.height);

	RECT realDrawDest = drawDest;
	{ // 根据图片等比例缩放 确认实际的渲染区域
		auto cx = drawDest.right - drawDest.left;
		auto cy = drawDest.bottom - drawDest.top;
		float wndRadio = float(cx) / float(cy);
		float frameRadio = float(texInfo.width) / float(texInfo.height);
		float radio;
		if (wndRadio > frameRadio) {
			radio = float(cy) / float(texInfo.height);
		} else {
			radio = float(cx) / float(texInfo.width);
		}
		auto destCx = radio * texInfo.width;
		auto destCy = radio * texInfo.height;
		if (wndRadio > frameRadio) {
			realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
			realDrawDest.right = realDrawDest.left + (LONG)destCx;
		} else {
			realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
			realDrawDest.bottom = realDrawDest.top + (LONG)destCy;
		}
	}

	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, realDrawDest, true, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));

	pGraphic->DrawTexture(shader, FilterType::FilterAnisotropic, texs);
}

void FillRectangle(SIZE canvas, RECT drawDest, ST_Color clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderFillRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, true, matrixWVP);

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
