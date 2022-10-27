#pragma once
#include <assert.h>
#include <map>
#include "DX11GraphicAPI.h"
#include "convert-video-yuv2rgb.h"

enum class ShaderType {
	shaderTexture = 0,
	shaderFillRect,
	yuv420ToRGB,
};

extern float matrixWVP[4][4];
extern IDX11GraphicInstance *pGraphic;
extern std::map<ShaderType, shader_handle> shaders;
extern std::shared_ptr<FormatConvert_YUVToRGB> pI4202RGB;

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);
void FillRect(SIZE canvas, RECT drawDest, ST_Color clr);
void RenderBorderWithSize(SIZE canvas, RECT drawDest, long borderSize, ST_Color clr);
void YUV_To_RGB(SIZE canvas, RECT drawDest);
