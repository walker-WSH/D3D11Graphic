
Texture2D shaderTexture;
SamplerState SampleType;
 
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};
 
float4 PS(PixelInputType input) : SV_TARGET
{ 
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);  
    textureColor.w = 1.0;
    return textureColor;
}