struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    unorm float4 Color : TEXCOORD1;
};

[[vk::binding(0, 0)]]
Texture2D texture : register(t0);
[[vk::binding(1, 0)]]
SamplerState texture_sampler : register(s0);

[shader("pixel")]
float4 main(VSOutput input) : SV_TARGET
{
    return input.Color + texture.Sample(texture_sampler, input.UV);
}