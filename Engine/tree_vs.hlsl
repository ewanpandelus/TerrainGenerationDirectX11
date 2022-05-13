
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float3 assignedColor;
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
    float3 assignedColor : COLOR;


};

OutputType main(InputType input)
{
    OutputType output;

    input.position.w = 1.0f;
    output.assignedColor = assignedColor;
    output.position = mul(input.position, worldMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);


    float amp = 2;
    float waveLength = 10;


    float3 p = output.position.xyz;

    p.y = sin(p.x);



    //output.position.xyz = p;
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    //Calculalte light position 
    float4 worldPos = mul(input.position, worldMatrix);


    //output.color = input.color;

// world position of vertex (for point light)
    output.position3D = (float3)mul(input.position, worldMatrix);

    return output;
}