#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <source_location>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

class DX11GraphicObject;
class IDX11GraphicSession;
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

enum class FilterType {
	FilterPoint = 0,
	FilterLinear,
	FilterAnisotropic,
};

enum class MapTextureType {
	MapRead = 0,
	MapWrite,
};

enum class VertexInputType {
	Normal = 0,
	Color,
	Positon,
	SVPosition,
	TextureCoord,
};

enum class BlendStateType {
	Disable = 0,
	Normal,
};

struct ST_TextureInfo {
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	enum TextureType usage = TextureType::Unknown;
};

struct ST_DisplayInfo {
	HWND hWnd = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
};

struct ST_Color {
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	float alpha = 0.0;
};

struct ST_VertexInputDesc {
	VertexInputType type;
	uint32_t size;
};

struct ST_ShaderInfo {
	std::wstring vsFile;
	std::wstring psFile;

	std::vector<ST_VertexInputDesc> vertexDesc;
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
	virtual void OnBuildSuccessed(const DXGI_ADAPTER_DESC &desc) = 0;
};

class GRAPHIC_API AutoGraphicContext {
public:
	AutoGraphicContext(IDX11GraphicSession *graphic, const std::source_location &location);
	virtual ~AutoGraphicContext();

private:
	class impl;
	impl *self;
};
