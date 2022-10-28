#include "convertToRGB-ps-base.hlsli"

float4 PS(VertexOut pIn) : SV_Target
{
	float3 rgb = ps_planar420_to_rgb(pIn);
	float4 res_color = float4(rgb, 1.0);
	return res_color;
}
