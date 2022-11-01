#pragma once
#include <assert.h>
#include <map>
#include "DX11GraphicAPI.h"
#include "convert-video-yuv2rgb.h"
#include "convert-video-rgb2yuv.h"

enum class ShaderType {
	shaderTexture = 0,
	shaderFillRect,

	// yuv to rgb
	i420ToRGB,
	nv12ToRGB,
	yuy2ToRGB,

	// rgb to yuv
	yuvOnePlane,
	uvPlane,
};

extern IDX11GraphicInstance *pGraphic;
extern std::map<ShaderType, shader_handle> shaders;

void InitShader();

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);
void FillRectangle(SIZE canvas, RECT drawDest, ST_Color clr);
void RenderBorderWithSize(SIZE canvas, RECT drawDest, long borderSize, ST_Color clr);
