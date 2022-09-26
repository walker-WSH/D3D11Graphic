#pragma once
#include <Windows.h>
#include <vector>
#include <assert.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <ComPtr.hpp>

const static IID DXGIFactory2 = {0x50c83a1c, 0xe072, 0x4c48, {0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0}};
const static std::vector<D3D_FEATURE_LEVEL> featureLevels = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
};
