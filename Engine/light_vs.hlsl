// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float time;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;

};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
    float time : TIME;
    float waveCol : WAVECOLOR;

};
float hash(float2 n)
{
    return frac(sin(dot(n, float2(123.456789, 987.654321))) * 54321.9876);
}
float noise(float2 p)
{
    float2 i = floor(p);
    float2 u = smoothstep(0.0, 1.0, frac(p));
    float a = hash(i + float2(0, 0));
    float b = hash(i + float2(1, 0));
    float c = hash(i + float2(0, 1));
    float d = hash(i + float2(1, 1));
    float r = lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
    return r * r;
}

float fbm(float2 p, int octaves)
{
    float value = 0.0;
    float amplitude = 0.5;
    float e = 3.0;
    for (int i = 0; i < octaves; ++i)
    {
        value += amplitude * noise(p);
        p = p * e;
        amplitude *= 0.5;
        e *= 0.9;
    }
    return value;
}
OutputType main(InputType input)
{

    OutputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);

   

    //output.position.y += fbm(cos(time*input.tex),4);
 

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    //Calculalte light position 
    float4 worldPos = mul(input.position, worldMatrix);


  
    // world position of vertex (for point light)
    output.position3D = (float3)mul(input.position, worldMatrix);

    return output;
}