cbuffer SkyboxConstants : register(b0)
{
    float4x4 WorldViewProj;
}
struct VSOutput
{
    float3 TexCoord : TEXCOORD0;
    float4 PositionPS : SV_Position;
};
VSOutput main(float4 position : SV_Position)
{
    VSOutput vout;

    vout.PositionPS = mul(position, WorldViewProj);
    vout.PositionPS.z = vout.PositionPS.w; // Draw on far plane

    vout.TexCoord.x = -position.x;
    vout.TexCoord.yz = position.yz;

    return vout;
}