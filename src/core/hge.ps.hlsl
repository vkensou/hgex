struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    unorm float4 Color : TEXCOORD1;
};

[shader("pixel")]
float4 main(VSOutput input) : SV_TARGET
{
    return input.Color;
}