#include "pch.h"
#include "NoiseFunctions.h"




NoiseFunctions::~NoiseFunctions()
{
   
}
void NoiseFunctions::Setup() {
    perlinNoise.Initialize();
}
   

void NoiseFunctions::PopulateRandomPoints(int width, int height, int numPoints, int scale) {
    randomPoints.clear();
    for (int pointNumber = 0; pointNumber < numPoints; pointNumber++) {
        randomPoints.push_back(SimpleMath::Vector2(rand() % width/scale, rand() % height/scale));
    }
}
float NoiseFunctions::VoronoiTesselation(int x, int y) {
    
    std::vector<float> distances; 
    for (int i = 0; i < randomPoints.size();i++) {
        distances.push_back(Distance(randomPoints[i], SimpleMath::Vector2(x, y)));
    }
    std::sort(distances.begin(), distances.end());
    return distances[0]/4;
}
float NoiseFunctions::Distance(SimpleMath::Vector2 a, SimpleMath::Vector2 b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    return sqrt((x * x) + (y * y));
}

float NoiseFunctions::RidgeNoise(float x, float y, float scale , float freq) {
    return (1 - abs(perlinNoise.Noise( x * scale * freq, y * scale* freq, 1)) - 0.5);
}
float NoiseFunctions::PerlinNoise(float x, float y, float scale, float freq) {
    return perlinNoise.Noise(x * scale * freq, y *scale * freq, 1);
}










