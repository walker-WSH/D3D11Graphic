SamplerState gSampler : register(s0);

Texture2D image0 : register(t0);
Texture2D image1 : register(t1);
Texture2D image2 : register(t2);
Texture2D image3 : register(t3);

cbuffer PSConstants : register(b0)
{
	float4 color_vec0;
	float4 color_vec1;
	float4 color_vec2;
	float4 color_range_min;
	float4 color_range_max;

	float width;
	float height;
	float width_d2;
	int full_range;
}

const static int VIDEO_FORMAT_I420 = 1;
const static int VIDEO_FORMAT_NV12 = 2;
const static int VIDEO_FORMAT_YVYU = 3;
const static int VIDEO_FORMAT_YUY2 = 4; /* YUYV */
const static int VIDEO_FORMAT_UYVY = 5;
const static int VIDEO_FORMAT_RGBA = 6;
const static int VIDEO_FORMAT_BGRA = 7;
const static int VIDEO_FORMAT_BGRX = 8;
const static int VIDEO_FORMAT_Y800 = 9;
const static int VIDEO_FORMAT_I444 = 10;
const static int VIDEO_FORMAT_BGR3 = 11;
const static int VIDEO_FORMAT_I422 = 12;
const static int VIDEO_FORMAT_I40A = 13;
const static int VIDEO_FORMAT_I42A = 14;
const static int VIDEO_FORMAT_YUVA = 15;
const static int VIDEO_FORMAT_AYUV = 16;

struct VertexOut {
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

float3 YUV_to_RGB(float3 yuv)
{
	yuv = clamp(yuv, color_range_min.xyz, color_range_max.xyz);
	float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
	float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
	float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
	return float3(r, g, b);
}

//-------------------------------------------------------------------
float3 ps_planar420_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_nv12_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float2 uv = image1.Sample(gSampler, pIn.uv).xy;
	float3 yuv = float3(y, uv);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_yvyu_to_rgb(VertexOut pIn)
{
	float4 yvyu = image0.Sample(gSampler, pIn.uv);
	float real_offset = pIn.uv.x * width_d2;
	float leftover = frac(real_offset);
	float y = leftover < 0.5 ? yvyu.x : yvyu.z;
	float3 yuv = float3(y, yvyu.wy);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_yuy2_to_rgb(VertexOut pIn)
{
	float4 yuyv = image0.Sample(gSampler, pIn.uv);
	float real_offset = pIn.uv.x * width_d2;
	float leftover = frac(real_offset);
	float y = leftover < 0.5 ? yuyv.z : yuyv.x;
	float3 yuv = float3(y, yuyv.yw);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_uyvy_to_rgb(VertexOut pIn)
{
	float4 uyvy = image0.Sample(gSampler, pIn.uv);
	float real_offset = pIn.uv.x * width_d2;
	float leftover = frac(real_offset);
	float y = leftover < 0.5 ? uyvy.y : uyvy.w;
	float3 yuv = float3(y, uyvy.zx);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_planar444_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float3 ps_bgr3_to_rgb(VertexOut pIn)
{
	int real_offset_x = int(pIn.uv.x * width);
	int pos_begin = real_offset_x * 3;
	int pos_y = pIn.uv.y * height;
	float b = image0.Load(int3(pos_begin, pos_y, 0)).x;
	float g = image0.Load(int3(pos_begin + 1, pos_y, 0)).x;
	float r = image0.Load(int3(pos_begin + 2, pos_y, 0)).x;
	float3 rgb = float3(r, g, b);
	if (full_range == 0) {
		rgb = (255.0 / 219.0) * rgb - (16.0 / 219.0);
	}
	return rgb;
}

float3 ps_planar422_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return rgb;
}

float4 ps_planar420_a_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float a = image3.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return float4(rgb, a);
}

float4 ps_planar422_a_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float a = image3.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return float4(rgb, a);
}

float4 ps_planar_yuva_to_rgb(VertexOut pIn)
{
	float y = image0.Sample(gSampler, pIn.uv).x;
	float u = image1.Sample(gSampler, pIn.uv).x;
	float v = image2.Sample(gSampler, pIn.uv).x;
	float a = image3.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return float4(rgb, a);
}

float4 ps_planar_ayuv_to_rgb(VertexOut pIn)
{
	float a = image0.Sample(gSampler, pIn.uv).x;
	float y = image1.Sample(gSampler, pIn.uv).x;
	float u = image2.Sample(gSampler, pIn.uv).x;
	float v = image3.Sample(gSampler, pIn.uv).x;
	float3 yuv = float3(y, u, v);
	float3 rgb = YUV_to_RGB(yuv);
	return float4(rgb, a);
}

float4 ps_rgba_to_rgba(VertexOut pIn)
{
	float4 color = image0.Sample(gSampler, pIn.uv).rgba;
	if (full_range == 0) {
		float3 rgb = (255.0 / 219.0) * color.xyz - (16.0 / 219.0);
		color = float4(rgb, color.a);
	}
	return color;
}

float4 ps_bgr_to_rgba(VertexOut pIn, bool alpha_valid)
{
	float4 color = image0.Sample(gSampler, pIn.uv).rgba;
	if (full_range == 0) {
		color.xyz = (255.0 / 219.0) * color.xyz - (16.0 / 219.0);
	}

	if (alpha_valid) {
		return color;
	} else {
		return float4(color.xyz, 1.0);
	}
}

float4 ps_y800_to_rgba(VertexOut pIn)
{
	float gray = image0.Sample(gSampler, pIn.uv).x;
	float4 color = float4(gray, gray, gray, 1.0);
	if (full_range == 0) {
		color.xyz = (255.0 / 219.0) * color.xyz - (16.0 / 219.0);
	}
	return color;
}

//-------------------------------------------------------------------
/*
float4 PS(VertexOut pIn) : SV_Target
{
	float4 res_color = float4(0.0, 0.0, 0.0, 0.0);

	int conv_type = VIDEO_FORMAT_I420;
	if (conv_type == VIDEO_FORMAT_I420) {
		float3 c = ps_planar420_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_NV12) {
		float3 c = ps_nv12_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_YVYU) {
		float3 c = ps_yvyu_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_YUY2) {
		float3 c = ps_yuy2_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_UYVY) {
		float3 c = ps_uyvy_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_I444) {
		float3 c = ps_planar444_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_BGR3) {
		float3 c = ps_bgr3_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_I422) {
		float3 c = ps_planar422_to_rgb(pIn);
		res_color = float4(c, 1.0);
	} else if (conv_type == VIDEO_FORMAT_I40A) {
		res_color = ps_planar420_a_to_rgb(pIn);
	} else if (conv_type == VIDEO_FORMAT_I42A) {
		res_color = ps_planar422_a_to_rgb(pIn);
	} else if (conv_type == VIDEO_FORMAT_YUVA) {
		res_color = ps_planar_yuva_to_rgb(pIn);
	} else if (conv_type == VIDEO_FORMAT_AYUV) {
		res_color = ps_planar_ayuv_to_rgb(pIn);
	} else if (conv_type == VIDEO_FORMAT_RGBA) {
		res_color = ps_rgba_to_rgba(pIn);
	} else if (conv_type == VIDEO_FORMAT_BGRA) {
		res_color = ps_bgr_to_rgba(pIn, true);
	} else if (conv_type == VIDEO_FORMAT_BGRX) {
		res_color = ps_bgr_to_rgba(pIn, false);
	} else if (conv_type == VIDEO_FORMAT_Y800) {
		res_color = ps_y800_to_rgba(pIn);
	}

	return res_color;
}
*/
