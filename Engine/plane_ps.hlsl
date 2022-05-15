// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);

float3 mod289(float3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float2 mod289(float2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float3 permute(float3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float random(float2 st)
{
    return frac(sin(dot(st.xy,
        float2(12.9898, 78.233))) *
        43758.5453123);
}
float snoise(float2 v) {

    // Precompute values for skewed triangular grid
    const float4 C = float4(0.211324865405187,
        // (3.0-sqrt(3.0))/6.0
        0.366025403784439,
        // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,
        // -1.0 + 2.0 * C.x
        0.024390243902439);
    // 1.0 / 41.0

// First corner (x0)
    float2 i = floor(v + dot(v, C.yy));
    float2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    float2 i1 = float2(0, 0);
    i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    float2 x1 = x0.xy + C.xx - i1;
    float2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    float3 p = permute(
        permute(i.y + float3(0.0, i1.y, 1.0))
        + i.x + float3(0.0, i1.x, 1.0));

    float3 m = max(0.5 - float3(
        dot(x0, x0),
        dot(x1, x1),
        dot(x2, x2)
        ), 0.0);

    m = m * m;
    m = m * m;

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    float3 x = 2.0 * frac(p * C.www) - 1.0;
    float3 h = abs(x) - 0.5;
    float3 ox = floor(x + 0.5);
    float3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    // Compute final noise value at P
    float3 g = float3(0, 0, 0);
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * float2(x1.x, x2.x) + h.yz * float2(x1.y, x2.y);
    return 130.0 * dot(m, g);
}



float ridge(float h, float offset) {
    h = abs(h);     // create creases
    h = offset - h; // invert so creases are at top
    h = h * h;      // sharpen creases
    return h;
}

float ridgedMF(float2 p) {
    float lacunarity = 2.0;
    float gain = 0.5;
    float offset = 0.9;

    float sum = 0.0;
    float freq = 1.0, amp = 0.5;
    float prev = 1.0;
    for (int i = 0; i < 5; i++) {
        float n = ridge(snoise(p * freq), offset);
        sum += n * amp;
        sum += n * amp * prev;  // scale by previous octave
        prev = n;
        freq *= lacunarity;
        amp *= gain;
    }
    return sum;
}

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

     color += lerp(float4(1,0,0,0), float4(1.0,0.9,0,0), ridgedMF(st * 3.0 + input.time/20));
     lightDir = normalize(input.position3D - lightPosition);

     // Calculate the amount of light on this pixel.
     lightIntensity = saturate(dot(input.normal, -lightDir));

     // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
     color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
     color = saturate(color);

     return float4(color, 1) * ridgedMF(st/20)* float4(0.2,0.2,1,1)*1.4;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.

}

