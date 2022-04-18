// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)
Texture2D grassTexture : register(t0);
Texture2D rockTexture : register(t1);
Texture2D slopeTexture : register(t2);
Texture2D snowTexture : register(t3);
Texture2D waterTexture: register(t4);
Texture2D sandTexture: register(t5);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightPosition;
	float padding;
};
cbuffer TerrainColourBuffer : register(b1)
{
	float4 bottomColour;
	float4 secondColour;
	float4 thirdColour;
	float4 topColour;
};
struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;

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
float InverseLerp(float xx, float yy, float value)
{
	return (value - xx) / (yy - xx);
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
float4  RockTexBlend(float slope, float4 textureColor, float4 grassColor, float4 slopeColor, float4 rockColor) {
	float blendAmount;
	if (slope < 0.2)
	{
		blendAmount = slope / 0.2f;
		textureColor = lerp(grassColor, slopeColor, blendAmount);
	}

	if ((slope < 0.7) && (slope >= 0.2f))
	{
		blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
		textureColor = lerp(slopeColor, rockColor, blendAmount);
	}

	if (slope >= 0.7)
	{
		textureColor = rockColor;
	}
	return textureColor;
}
float4 ScaleTexAllDirections(Texture2D _tex, float3 scaledWorldPos, float3 blendAxes) {
	float3 xProj = _tex.Sample(SampleType, scaledWorldPos.xy*4) * blendAxes.x;
	float3 yProj = _tex.Sample(SampleType, scaledWorldPos.xz*4) * blendAxes.y;
	float3 zProj = _tex.Sample(SampleType, scaledWorldPos.xy*4) * blendAxes.z;
	return float4((xProj+yProj+zProj)/3, 1);
}
float4  SnowTexBlend(float slope, float4 textureColor, float4 grassColor, float4 slopeColor, float4 snowColor) {
	float blendAmount;
	if (slope < 0.2)
	{
		blendAmount = slope / 0.2f;
		textureColor = lerp(grassColor, slopeColor, 0.01);
	}

	if ((slope < 0.7) && (slope >= 0.2f))
	{
		blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
		textureColor = lerp(snowColor, slopeColor, blendAmount);
	}

	if (slope >= 0.7)
	{
		textureColor = lerp(snowColor, slopeColor, 0.9);
	}
	return textureColor;
}

float4 TwoTextureBlendByHeight(float4 tex1, float4 tex2, float offset, float rangeTo1, float y) {
	return lerp(tex1, tex2, saturate((y + offset)*(1/rangeTo1)));
}
float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
	float3	lightDir;
	float	lightIntensity;
	float4	color;

	float4 grassTex;
	float4 slopeTex;
	float4 rockTex;
	float4 snowTex;
	float4 waterTex;
	float4 sandTex;
 
	float4 darkWaterColour;
	float4 lightWaterColour;
	float4 sandColour; 


	float3 blendAxes = abs(input.normal);
	blendAxes /= (blendAxes.x + blendAxes.y + blendAxes.z);

	grassTex = grassTexture.Sample(SampleType, input.tex) *bottomColour;
	//slopeTex = ScaleTexAllDirections(slopeTexture, input.position3D, blendAxes)*secondColour;
	slopeTex = slopeTexture.Sample(SampleType, input.tex)  *secondColour;
	//rockTex = ScaleTexAllDirections(rockTexture, input.position3D, blendAxes) * secondColour;
	rockTex = rockTexture.Sample(SampleType, input.tex/2)   *secondColour;
	snowTex = snowTexture.Sample(SampleType, input.tex) * topColour;
	waterTex = waterTexture.Sample(SampleType, input.tex / 5);
	sandTex = sandTexture.Sample(SampleType, input.tex/2)* thirdColour;

	darkWaterColour = waterTex * float4(0.4, 0.4, 0.4, 1);
	lightWaterColour = waterTex * float4(1.0, 1.0, 1.0, 1.0);
	





	float slope;

	   
	// Calculate the slope of this point.
	slope = 1 - input.normal.y;
	


	float4 grassSlope = RockTexBlend(slope, textureColor, grassTex, slopeTex, rockTex);          //lerp((grassTex), slopeTex, slope + 0.15);
	float4 snowSlope = RockTexBlend(slope, textureColor, snowTex, slopeTex, rockTex);

	// Calculate the slope of this point.
	if (input.position3D.y < -0.7) {
		textureColor = darkWaterColour;
	}
	if (input.position3D.y >= -0.7 && input.position3D.y < -0.4) {
		textureColor = lerp(darkWaterColour, lightWaterColour, (input.position3D.y+0.7)*3.33);
	}
	if (input.position3D.y >= -0.4 && input.position3D.y <= -0.35) {
		textureColor = lerp(lightWaterColour, sandTex, (input.position3D.y + 0.4) * 20);
	}
	if (input.position3D.y < -0.1&& input.position3D.y >-0.35) {
		textureColor = lerp(sandTex, grassSlope, (input.position3D.y + 0.35)*4);
		
	}
	if (input.position3D.y <=0.5 && input.position3D.y >= -0.1) {
		textureColor = lerp(grassSlope, snowSlope, (input.position3D.y + 0.1) * 1.67);
	}
	if (input.position3D.y > 0.5) {
		textureColor = snowSlope;
	}
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);
	


	//return snowTex;
	return  color * textureColor;
}