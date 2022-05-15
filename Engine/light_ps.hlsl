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
	float3 steepSlopeColour;
	float4 wonGame;
};
struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;

};
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
	slopeTex *= float4(steepSlopeColour,1);
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
	


	float2 uvsCentred = (input.tex/(6+cos(padding/10)*4));
	float radialDistance = length(uvsCentred);


	

	float3 rings = cos((radialDistance * lerp(input.normal, 1-input.normal, radialDistance) - padding * 0.01) * 3.14 * 10);


	float f = ridgedMF(input.tex/4 + padding/20);

	
	float4 fadeBetweenNormalAndInverted = saturate(float4(lerp(input.normal, 1 - input.normal, cos(padding)), 1) * f);
	float4 result = lerp(fadeBetweenNormalAndInverted, float4(rings, 1), 0.4);
	

	float4 terrainColour = color * textureColor;

	float4 psychadelicIfWon = lerp(terrainColour, terrainColour * result, wonGame.x);

	return psychadelicIfWon;
}