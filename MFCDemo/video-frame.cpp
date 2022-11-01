#include "pch.h"
#include <assert.h>
#include "video-frame.h"

#if 1
int width = 1920;
int height = 1080;

int lenY = width * height;
int lenU = (width / 2) * (height / 2);
int lenV = (width / 2) * (height / 2);

uint8_t *i420Y = new uint8_t[lenY];
uint8_t *i420U = new uint8_t[lenU];
uint8_t *i420V = new uint8_t[lenV];

AVFrame *frame_i420 = nullptr;
AVFrame *frame_yuy2 = nullptr;

bool readVideo()
{
	FILE *fp = nullptr;
	fopen_s(&fp, "1080p.i420", "rb+");
	if (!fp) {
		assert(false);
		return false;
	}

	fread(i420Y, lenY, 1, fp);
	fread(i420U, lenU, 1, fp);
	fread(i420V, lenV, 1, fp);

	fclose(fp);
	return true;
}

bool initVideo_i420()
{
	if (!readVideo())
		return false;

	frame_i420 = av_frame_alloc();
	frame_i420->format = AV_PIX_FMT_YUV420P;
	frame_i420->width = width;
	frame_i420->height = height;

	av_frame_get_buffer(frame_i420, 32);
	memmove(frame_i420->data[0], i420Y, lenY);
	memmove(frame_i420->data[1], i420U, lenU);
	memmove(frame_i420->data[2], i420V, lenV);

	return true;
}

bool initVideo_yuy2()
{
	frame_yuy2 = av_frame_alloc();
	frame_yuy2->format = AV_PIX_FMT_YUYV422;
	frame_yuy2->width = 1920;
	frame_yuy2->height = 1080;
	av_frame_get_buffer(frame_yuy2, 32);

	FILE *fp = nullptr;
	fopen_s(&fp, "1080p.yuy2", "rb+");
	assert(fp);
	if (fp) {
		fread(frame_yuy2->data[0], frame_yuy2->width * frame_yuy2->height * 2, 1, fp);
		fclose(fp);
	}

	return true;
}
#endif

//------------------------------------------------------------------------------------
AVFormatContext *input_ctx = nullptr;
AVCodecContext *decoder_ctx = nullptr;
AVStream *video = nullptr;
int video_stream = 0;
struct SwsContext *sws_ctx = nullptr;

int open_file()
{
	initVideo_i420();
	initVideo_yuy2();

	int ret;

	/* open the input file */
	if (avformat_open_input(&input_ctx, "test.wmv", NULL, NULL) != 0) {
		return -1;
	}

	if (avformat_find_stream_info(input_ctx, NULL) < 0) {
		return -1;
	}

	/* find the video stream information */
	AVCodec *test = NULL;
	ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &test, 0);
	if (ret < 0) {
		return -1;
	}

	video_stream = ret;
	video = input_ctx->streams[video_stream];

	AVCodecParameters *const codecpar = video->codecpar;
	const AVCodec *const decoder =
		avcodec_find_decoder(codecpar->codec_id); // fix discarded-qualifiers

	decoder_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(decoder_ctx, codecpar);

	if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
		return -1;

	if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
		return -1;
	}

	return 0;
}

AVFrame *decode_frame()
{
	AVPacket packet = {0};
	int ret;

	while (true) {
		ret = av_read_frame(input_ctx, &packet);
		if (ret < 0) {
			return nullptr;
		}

		if (video_stream = packet.stream_index)
			break;

		av_packet_unref(&packet);
	}

	AVFrame *frame = av_frame_alloc();
	int got_frame = 0;

	while (!got_frame) {
		ret = avcodec_send_packet(decoder_ctx, &packet);
		if (ret == 0)
			ret = avcodec_receive_frame(decoder_ctx, frame);

		got_frame = (ret == 0);

		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
			ret = 0;

		if (ret < 0) {
			break;
		}
	}

	av_packet_unref(&packet);

	if (got_frame) {
		if (!sws_ctx) {
			sws_ctx = sws_getContext(frame->width, frame->height,
						 (enum AVPixelFormat)frame->format, frame->width,
						 frame->height, destFormat, SWS_BICUBIC, NULL, NULL,
						 NULL);
			assert(sws_ctx);
		}

		AVFrame *output = av_frame_alloc();
		output->format = destFormat;
		output->width = frame->width;
		output->height = frame->height;

		int ret = av_frame_get_buffer(output, 1);
		if (ret < 0) {
			assert(false);
			return nullptr;
		}

		sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
			  frame->height, output->data, output->linesize);

		av_frame_free(&frame);
		return output;

	} else {
		av_frame_free(&frame);
		return nullptr;
	}
}
