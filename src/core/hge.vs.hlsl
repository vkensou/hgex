struct VSInput
{
    [[vk::location(0)]]
    float3 Pos : POSITION;
    [[vk::location(1)]]
    unorm float4 Color : COLOR;
    [[vk::location(2)]]
    float2 UV : TEXCOORD0;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    unorm float4 Color : TEXCOORD1;
};

[[vk::binding(0, 1)]]
cbuffer PerFrame
{
    float4x4 matProj;
    float4x4 matView;
} 

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Pos = mul(matProj, mul(matView, float4(input.Pos, 1.0)));
    output.UV = input.UV;
    output.Color = input.Color;
    return output;
}
