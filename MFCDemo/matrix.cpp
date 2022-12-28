#include "pch.h"
#include "matrix.h"
#include <dxsdk/include/d3dx10math.h>

// dxsdk
#pragma comment(lib, "D3DX10.lib")

D3DXMATRIX GetWorldMatrix(float scaleX, float scaleY, float left, float top)
{
	D3DXMATRIX outputWorldMatrix;
	D3DXMatrixIdentity(&outputWorldMatrix);

	D3DXMATRIX rotate;
	D3DXMatrixIdentity(&rotate);
	rotate.m[0][0] = 0;
	rotate.m[1][1] = 0;
	rotate.m[0][1] = 1.f;
	rotate.m[1][0] = 1.f;
	//outputWorldMatrix *= rotate;

	D3DXMATRIX scale;
	D3DXMatrixIdentity(&scale);
	scale.m[0][0] = scaleX;
	scale.m[1][1] = scaleY;
	outputWorldMatrix *= scale;

	D3DXMATRIX mv;
	D3DXMatrixIdentity(&mv);
	mv.m[3][0] = left;
	mv.m[3][1] = top;
	outputWorldMatrix *= mv;

	outputWorldMatrix.m[0][2] = -outputWorldMatrix.m[0][2];
	outputWorldMatrix.m[1][2] = -outputWorldMatrix.m[1][2];
	outputWorldMatrix.m[2][2] = -outputWorldMatrix.m[2][2];
	outputWorldMatrix.m[3][2] = -outputWorldMatrix.m[3][2];

	return outputWorldMatrix;
}

D3DXMATRIX GetOrthoMatrix(SIZE canvas)
{
	FLOAT zn = -100.f;
	FLOAT zf = 100.f;

	D3DXMATRIX orthoMatrix;
	D3DXMatrixOrthoLH(&orthoMatrix, (float)canvas.cx, (float)canvas.cy, zn, zf);

	orthoMatrix.m[1][1] = -orthoMatrix.m[1][1];
	orthoMatrix.m[3][0] = -1.0f;
	orthoMatrix.m[3][1] = 1.0f;

	return orthoMatrix;
}

void TransposeMatrixWVP(SIZE canvas, SIZE texture, RECT destPos, TextureRenderMode mode,
			float outputMatrix[4][4])
{
	float scaleX = float(destPos.right - destPos.left) / float(texture.cx);
	float scaleY = scaleX;

	switch (mode) {
	case TextureRenderMode::FitToRect:
		scaleY = scaleX;
		break;

	case TextureRenderMode::FullCoverRect:
	default:
		scaleY = float(destPos.bottom - destPos.top) / float(texture.cy);
		break;
	}

	D3DXMATRIX worldMatrix =
		GetWorldMatrix(scaleX, scaleY, (float)destPos.left, (float)destPos.top);
	D3DXMATRIX orthoMatrix = GetOrthoMatrix(canvas);

	D3DXMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	D3DXMatrixTranspose(&wvpMatrix, &wvpMatrix);

	void *src = &(wvpMatrix.m[0][0]);
	memmove(&(outputMatrix[0][0]), src, sizeof(float) * 16);
}
