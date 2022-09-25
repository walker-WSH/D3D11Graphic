#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <source_location>

class DX11GraphicObject;
class IDX11GraphicInstance;
class AutoGraphicContext;

using texture_handle = DX11GraphicObject *;
using display_handle = DX11GraphicObject *;
using shader_handle = DX11GraphicObject *;

enum class TextureType {
	Unknown = 0,
	CanvasTarget,
	ReadTexture,
	WriteTexture,
	SharedHandle,
	StaticImageFile,
};

struct ST_TextureInfo {
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	enum TextureType usage = TextureType::Unknown;
};

struct ST_TextureVertex {
	float x, y, z, w;
	float u, v;
};

struct ST_Color {
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	float alpha = 0.0;
};

struct ST_ShaderInfo {
	std::wstring vsFile;
	std::wstring psFile;

	uint32_t perVertexSize = 0;
	uint32_t vertexCount = 0;

	uint32_t vsBufferSize = 0;
	uint32_t psBufferSize = 0;
};

struct ST_GraphicCardInfo {
	std::wstring graphicName = L"";
	std::string driverVersion = "";

	LUID adapterLuid = {0, 0};
	uint32_t vendorId = 0;
	uint32_t deviceId = 0;

	uint64_t dedicatedVideoMemory = 0;
	uint64_t dedicatedSystemMemory = 0;
	uint64_t sharedSystemMemory = 0;
};

class DX11GraphicObject {
public:
	virtual ~DX11GraphicObject() = default;
	virtual bool IsBuilt() = 0;
};

class DX11GraphicCallback {
public:
	virtual ~DX11GraphicCallback() = default;
	virtual void OnD3D11Error(HRESULT hr) = 0;
	virtual void OnDeviceRemoved() = 0;
	virtual void OnBuildSuccessed() = 0;
};

class __declspec(dllexport) AutoGraphicContext {
public:
	AutoGraphicContext(IDX11GraphicInstance *graphic, const std::source_location &location);
	virtual ~AutoGraphicContext();

private:
	class impl;
	impl *self;
};
