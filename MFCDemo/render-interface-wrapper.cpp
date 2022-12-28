#include "pch.h"
#include "render-interface-wrapper.h"

IDX11GraphicSession *pGraphic = nullptr;
std::map<ShaderType, shader_handle> shaders;

std::wstring GetShaderDirectory();
void InitShader()
{
	float matrixWVP[4][4];
	ST_ShaderInfo shaderInfo;

	ST_VertexInputDesc desc;
	desc.type = VertexInputType::SVPosition;
	desc.size = 16;
	shaderInfo.vertexDesc.push_back(desc);

	desc.type = VertexInputType::TextureCoord;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"default-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = 0;
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::shaderTexture] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"fill-rect-vs.cso";
		shaderInfo.psFile = dir + L"fill-rect-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(ST_Color);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::shaderFillRect] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"rgb-to-y-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(toyuv_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::yuvOnePlane] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"rgb-to-uv-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(toyuv_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::uvPlane] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"i420-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::i420ToRGB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"nv12-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::nv12ToRGB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"yuy2-to-rgb-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[ShaderType::yuy2ToRGB] = shader;
	}
}

extern bool bFullscreenCrop;
void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ST_TextureInfo texInfo = pGraphic->GetTextureInfo(texs.at(0));
	shader_handle shader = shaders[ShaderType::shaderTexture];
	SIZE texSize(texInfo.width, texInfo.height);

	RECT realDrawDest = drawDest;
	float cropL = 0;
	float cropT = 0;
	float cropR = 0;
	float cropB = 0;
	if (!bFullscreenCrop) // fit to screen
	{
		// 根据图片等比例缩放 确认实际的渲染区域
		auto cx = drawDest.right - drawDest.left;
		auto cy = drawDest.bottom - drawDest.top;
		float wndRadio = float(cx) / float(cy);
		float frameRadio = float(texInfo.width) / float(texInfo.height);

		// 确认是按照宽度缩放 还是按照高度缩放
		if (wndRadio > frameRadio) {
			float radio = float(cy) / float(texInfo.height);
			auto destCx = radio * texInfo.width;

			realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
			realDrawDest.right = realDrawDest.left + (LONG)destCx;
		} else {
			float radio = float(cx) / float(texInfo.width);
			auto destCy = radio * texInfo.height;

			realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
			realDrawDest.bottom = realDrawDest.top + (LONG)destCy;
		}
	} else { // crop to ensure fullscreen

		// 根据图片等比例缩放 确认实际的渲染区域
		auto cx = drawDest.right - drawDest.left;
		auto cy = drawDest.bottom - drawDest.top;
		float wndRadio = float(cx) / float(cy);
		float frameRadio = float(texInfo.width) / float(texInfo.height);

		// 确认是按照宽度缩放 还是按照高度缩放
		if (wndRadio < frameRadio) {
			float radio = float(cy) / float(texInfo.height);
			auto destCx = radio * texInfo.width;

			realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
			realDrawDest.right = realDrawDest.left + (LONG)destCx;

			cropL = cropR = float(abs(realDrawDest.left - drawDest.left)) /
					float(realDrawDest.right - realDrawDest.left);
		} else {
			float radio = float(cx) / float(texInfo.width);
			auto destCy = radio * texInfo.height;

			realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
			realDrawDest.bottom = realDrawDest.top + (LONG)destCy;

			cropT = cropB = float(abs(realDrawDest.top - drawDest.top)) /
					float(realDrawDest.bottom - realDrawDest.top);
		}
	}

	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, realDrawDest, TextureRenderMode::FitToRect, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, cropL, cropT, cropR, cropB, outputVertex);

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));

	pGraphic->DrawTexture(shader, FilterType::FilterLinear, texs);
}

void FillRectangle(SIZE canvas, RECT drawDest, ST_Color clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderFillRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, TextureRenderMode::FullCoverRect, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, 0, 0, 0, 0, outputVertex);

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

std::wstring GetShaderDirectory()
{
	WCHAR szFilePath[MAX_PATH] = {};
	GetModuleFileNameW(0, szFilePath, MAX_PATH);

	int nLen = (int)wcslen(szFilePath);
	for (int i = nLen - 1; i >= 0; --i) {
		if (szFilePath[i] == '\\') {
			szFilePath[i + 1] = 0;
			break;
		}
	}

	auto ret = std::wstring(szFilePath) + L"HLSL\\";
	return ret;
}
