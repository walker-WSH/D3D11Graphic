
cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexInput {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

struct VertexOutput {
	float4 position : SV_POSITION;
};

VertexOutput VS(VertexInput input)
{
	VertexOutput output;
	output.position = mul(float4(input.position.xyz, 1.f), wvp);
	return output;
}
