struct UniformBufferObject
{
    float4x4 MVP;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBufferObject> ubo;

[[vk::binding(1, 0)]]
Texture2D texBaseColor;

[[vk::binding(2, 0)]]
SamplerState linearSampler;

// ----- VERTEX -----

struct VSInput
{
    float3 position : LOCATION0;
    float3 color    : LOCATION1;
    float2 uv       : LOCATION2;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : LOCATION0;
    float2 uv       : LOCATION1;
};

[shader("vertex")]
VSOutput vsMain(VSInput input)
{
    VSOutput o;
    o.position = mul(ubo.MVP, float4(input.position, 1.0));
    o.color = input.color;
    o.uv = input.uv;
    return o;
}

// ----- FRAGMENT -----

struct FSInput
{
    float3 color : LOCATION0;
    float2 uv    : LOCATION1;
};

[shader("fragment")]
float4 fsMain(FSInput input) : SV_Target0
{
    float3 texColor = texBaseColor.Sample(linearSampler, input.uv).rgb;

    return float4(input.color * texColor, 1.0);
}
