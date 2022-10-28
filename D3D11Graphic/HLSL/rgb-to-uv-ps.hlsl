SamplerState SampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer PSConstants : register(b0)
{
	float4 color_vec_u;
	float4 color_vec_v;
}

struct VertexOut {
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 PS(VertexOut pIn) : SV_Target
{
	float4 rgba = image0.Sample(SampleType, pIn.uv);
	float u = dot(color_vec_u.xyz, rgba.xyz) + color_vec_u.w;
	float v = dot(color_vec_v.xyz, rgba.xyz) + color_vec_v.w;
	return float4(u, v, 0.0, 1.0);
}
