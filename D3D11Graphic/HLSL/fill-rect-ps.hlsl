
cbuffer ColorBuffer
{
	float4 color;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};
 
float4 PS(PixelInputType input) : SV_TARGET
{ 
    return color;
}
