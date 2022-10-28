SamplerState SampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer PSConstants : register(b0)
{
	float4 color_vec;
	float4 reserve;
}

struct VertexOut {
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 PS(VertexOut pIn) : SV_Target
{
	float4 rgba = image0.Sample(SampleType, pIn.uv);
	float y = dot(color_vec.xyz, rgba.xyz) + color_vec.w;
	return float4(y, 0, 0, 0);
}
