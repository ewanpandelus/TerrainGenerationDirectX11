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
	void PopulateRandomPoints(int width, int height, int numPoints, int scale);
	float VoronoiTesselation(int x, int y);


private:
	float Distance(SimpleMath::Vector2 a, SimpleMath::Vector2 b);
};

