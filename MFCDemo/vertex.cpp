#include "pch.h"
#include "vertex.h"

void SwapFloat(float &f1, float &f2)
{
	float temp = f1;
	f1 = f2;
	f2 = temp;
}

void FillTextureVertex(SIZE texture, bool flipH, bool flipV, float cropLeft, float cropTop,
			     float cropRight, float cropBtm,
			     ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT])
{
	// 渲染的目标区域也是被裁剪后的区域 裁剪了的区域则什么都没画
	float left = 0 + cropLeft * texture.cx;
	float top = 0 + cropTop * texture.cy;
	float right = (float)texture.cx - cropRight * texture.cx;
	float bottom = (float)texture.cy - cropBtm * texture.cy;

	// 纹理采样的区域
	float leftUV = 0.f + cropLeft;
	float topUV = 0.f + cropTop;
	float rightUV = 1.f - cropRight;
	float bottomUV = 1.f - cropBtm;

	if (flipH)
		SwapFloat(leftUV, rightUV);

	if (flipV)
		SwapFloat(topUV, bottomUV);

	outputVertex[0] = {left, top, 0, 1.f, leftUV, topUV};
	outputVertex[1] = {right, top, 0, 1.f, rightUV, topUV};
	outputVertex[2] = {left, bottom, 0, 1.f, leftUV, bottomUV};
	outputVertex[3] = {right, bottom, 0, 1.f, rightUV, bottomUV};
}
