
struct PixelInputType
{
    float4 position : SV_POSITION;
};
 
float4 PS(PixelInputType input) : SV_TARGET
{ 
    return float4(1.0, 1.0, 1.0, 1.0);
}