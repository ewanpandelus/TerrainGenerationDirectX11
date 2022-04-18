#pragma once
#include "pch.h"
#include "PerlinNoise.h";



using namespace DirectX;

class NoiseFunctions
{
private:
	PerlinNoise perlinNoise;
	std::vector<SimpleMath::Vector2> randomPoints;
	

public:

	~NoiseFunctions();
	void Setup();
	float PerlinNoise(float x, float y, float scale, float freq);
	float RidgeNoise(float x, float y, float scale, float freq);
	float snoise(SimpleMath::Vector2 v);
	float RidgedMF(SimpleMath::Vector2 p);
	void PopulateRandomPoints(int width, int height, int numPoints, int scale);
	float VoronoiTesselation(int x, int y);


private:
	float Ridge(float h, float offset);

	

	float Distance(SimpleMath::Vector2 a, SimpleMath::Vector2 b);

	SimpleMath::Vector3 Floor3(SimpleMath::Vector3 a);
	SimpleMath::Vector3 Mod289(SimpleMath::Vector3 a);
	SimpleMath::Vector2 Mod289(SimpleMath::Vector2 a);
	SimpleMath::Vector2 Floor2(SimpleMath::Vector2 a);

	float Frac(float v);
	SimpleMath::Vector2 Frac2(SimpleMath::Vector2 st);
	SimpleMath::Vector3 Frac3(SimpleMath::Vector3 st);
	float Random(SimpleMath::Vector2 st);
	float Dot(SimpleMath::Vector2 a, SimpleMath::Vector2 b);
	float Dot3(SimpleMath::Vector3 a, SimpleMath::Vector3 b);
	float Noise(SimpleMath::Vector2 st);
	
	float  Lerp(float a, float b, float w);
	float Max(float a, float b);
	SimpleMath::Vector3 Permute(SimpleMath::Vector3 x);
	SimpleMath::Vector3 Abs3(SimpleMath::Vector3 x);
};

