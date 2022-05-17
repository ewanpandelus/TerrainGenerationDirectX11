// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 position3D : TEXCOORD2;
    float time : TIME;

};

float4 main(InputType input) : SV_TARGET
{
    float3	lightDir;
    float	lightIntensity;

    float4	textureColor;
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    float2 st = input.tex.xy;

    float3 color = float3(0, 0, 0);


    lightDir = normalize(input.position3D - lightPosition);

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, -lightDir));

    // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
    color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
    color = saturate(color);
    float flashEffect = (cos(input.time * 4) * 0.5);

    return float4(color, 1) * float4(1,0.5,0,1) * (flashEffect + 0.8);

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.

}

