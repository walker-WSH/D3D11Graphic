#pragma once
#include <Windows.h>
#include <stdint.h>
#include <memory>

#define MAX_VIDEO_PLANAR 3

enum class E_VideoFrameType {
	FT_RawData = 0,
	FT_Texture,
};

enum class E_VideoFormat {
	VF_Unkown = 0,

	VF_ARGB,
	VF_BGRA,

	// YUV420
	VF_I420,
	VF_NV12,
	VF_YV12,

	// YUV422
	VF_YUY2,
	VF_UYVY,
};

class IVideoFrame {
public:
	virtual ~IVideoFrame() = default;
	virtual E_VideoFrameType GetFrameType() = 0;

	int32_t nWidth = 0;
	int32_t nHeight = 0;
	E_VideoFormat eFormat = E_VideoFormat::VF_Unkown;
	int64_t n64Timestamp = 0; // in us

	bool bInterlaced = false;
	bool bFlipV = false;
};

class VideoFrameRaw : public IVideoFrame {
public:
	virtual E_VideoFrameType GetFrameType() { return E_VideoFrameType::FT_RawData; }

	uint8_t *aData[MAX_VIDEO_PLANAR] = {nullptr};
	int32_t aLinesize[MAX_VIDEO_PLANAR] = {0};
};

class VideoFrameTexture : public IVideoFrame {
public:
	virtual E_VideoFrameType GetFrameType() { return E_VideoFrameType::FT_Texture; }

	HANDLE hSharedHandle = 0;
	LUID adapterLuid = {0};
};

using VIDEO_FRAME = std::shared_ptr<IVideoFrame>;
