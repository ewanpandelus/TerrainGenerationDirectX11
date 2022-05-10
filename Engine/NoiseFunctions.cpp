#include "pch.h"
#include "NoiseFunctions.h"




NoiseFunctions::~NoiseFunctions()
{
   
}
void NoiseFunctions::Setup() {
    perlinNoise.Initialize();
}
   
SimpleMath::Vector3 NoiseFunctions::Floor3(SimpleMath::Vector3 v) {
    SimpleMath::Vector3 rv;
    int i;
    rv.x = v.x - Frac(v.x);
    rv.y = v.y - Frac(v.y);
    rv.z = v.z - Frac(v.z);
    return rv;
}
SimpleMath::Vector2 NoiseFunctions::Floor2(SimpleMath::Vector2 v) {
		SimpleMath::Vector2 rv;
		int i;
		rv.x = v.x - Frac(v.x);
		rv.y = v.y - Frac(v.y);
		return rv;
}
float NoiseFunctions::Frac(float v)
{
	return v - floor(v);
}
SimpleMath::Vector2 NoiseFunctions::Frac2(SimpleMath::Vector2 st) {
	SimpleMath::Vector2 rv;
	rv.x = Frac(st.x);
	rv.y = Frac(st.y);
	return rv;
}
SimpleMath::Vector3 NoiseFunctions::Frac3(SimpleMath::Vector3 st) {
    SimpleMath::Vector3 rv;
    rv.x = Frac(st.x);
    rv.y = Frac(st.y);
    rv.z = Frac(st.z);
    return rv;
}
float NoiseFunctions::Random(SimpleMath::Vector2 st) {
	return Frac(sin(Dot(st,
		SimpleMath::Vector2(12.9898, 78.233))) *
		43758.5453123);
}
float NoiseFunctions::Dot(SimpleMath::Vector2 a, SimpleMath::Vector2 b) {
	return a.x * b.x + a.y * b.y;
}
float NoiseFunctions::Dot3(SimpleMath::Vector3 a, SimpleMath::Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float NoiseFunctions::Ridge(float h, float offset) {
    h = abs(h);     // create creases
    h = offset - h; // invert so creases are at top
    h = h * h;      // sharpen creases
    return h;
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

float NoiseFunctions::Max(float a, float b) {
    if (a > b) return a;
    return b;
}
SimpleMath::Vector3 NoiseFunctions::Mod289(SimpleMath::Vector3 x) { return x - Floor3(x * (1.0 / 289.0)) * 289.0; }
SimpleMath::Vector3 NoiseFunctions::Permute(SimpleMath::Vector3 x) { return Mod289(((x * 34.0) + SimpleMath::Vector3(1.0,1.0,1.0)) * x); }

SimpleMath::Vector2 NoiseFunctions::Mod289(SimpleMath::Vector2 x) { return x - Floor2(x * (1.0 / 289.0)) * 289.0; }
SimpleMath::Vector3 NoiseFunctions::Abs3(SimpleMath::Vector3 x) {
    return SimpleMath::Vector3(abs(x.x), abs(x.y), abs(x.z));}







