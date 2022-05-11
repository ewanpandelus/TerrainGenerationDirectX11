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
	float4 sandColour;
	float4 grassColour;
	float4 mellowSlopeColour;
	float4 snowColour;
};
cbuffer TerrainExtraVariablesBuffer : register(b2)
{
	float4 overwritesColour;
	float4 waterColour;
	float4 steepSlopeColour;
	float pad2;
};
struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;

};

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

float4 OverwriteColour(float4 texColour) {
	float average = (texColour.x + texColour.y + texColour.z) / 3.0;
	return float4(average, average, average, 1);
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



	float3 blendAxes = abs(input.normal);
	blendAxes /= (blendAxes.x + blendAxes.y + blendAxes.z);

	grassTex = grassTexture.Sample(SampleType, input.tex);
	slopeTex = slopeTexture.Sample(SampleType, input.tex);
	rockTex = rockTexture.Sample(SampleType, input.tex / 2);
	snowTex = snowTexture.Sample(SampleType, input.tex);
	waterTex = waterTexture.Sample(SampleType, input.tex / 5);
	sandTex = sandTexture.Sample(SampleType, input.tex / 2);

	if (overwritesColour.x == 1) 
	{
		grassTex = OverwriteColour(grassTex);
		slopeTex = OverwriteColour(slopeTex);
		rockTex  = OverwriteColour(rockTex);
		snowTex  = OverwriteColour(snowTex);
		waterTex = OverwriteColour(waterTex);
		sandTex  = OverwriteColour(sandTex);
	}
	grassTex *= grassColour;
	slopeTex *= steepSlopeColour;
	rockTex  *= mellowSlopeColour;
	snowTex  *= snowColour;
	waterTex *= waterColour;
	sandTex  *= sandColour;

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
	if (input.position3D.y >= -0.7 && input.position3D.y < -0.1) {
		textureColor = lerp(darkWaterColour, lightWaterColour, (input.position3D.y+0.7)*1/0.66);
	}
	if (input.position3D.y >= -0.1 && input.position3D.y <= 0) {
		textureColor = lerp(lightWaterColour, sandTex, (input.position3D.y + 0.1) * 10);
	}
	if (input.position3D.y < 2&& input.position3D.y >0) {
		textureColor = lerp(sandTex, grassSlope, (input.position3D.y)/2);
		
	}
	if (input.position3D.y <=4 && input.position3D.y >= 2) {
		textureColor = lerp(grassSlope, snowSlope, (input.position3D.y - 2)/2);
	}
	if (input.position3D.y > 4) {
		textureColor = snowSlope;
	}
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);
	

	return  color * textureColor;
}