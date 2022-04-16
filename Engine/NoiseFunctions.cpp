#include "pch.h"
#include "NoiseFunctions.h"




NoiseFunctions::~NoiseFunctions()
{

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
float NoiseFunctions::Noise(SimpleMath::Vector2 st) {
	SimpleMath::Vector2 i = Floor2(st);
	SimpleMath::Vector2 f = Frac2(st);

	// Four corners in 2D of a tile
	float a = Random(i);
	float b = Random(i + SimpleMath::Vector2(1.0,0));
	float c = Random(i + SimpleMath::Vector2(0.0,1.0));
	float d = Random(i + SimpleMath::Vector2(1.0,1.0));

	SimpleMath::Vector2 u = f * f * SimpleMath::Vector2((3.0 - 2.0 * f.x), (3.0 - 2.0 * f.y));

	return Lerp(a, b, u.x) +
		(c - a) * u.y * (1.0 - u.x) +
		(d - b) * u.x * u.y;
}
float NoiseFunctions::Lerp(float a, float b, float w)
{
	return a + w * (b - a);
}
float NoiseFunctions::snoise(SimpleMath::Vector2 v) {

    // Precompute values for skewed triangular grid
    const SimpleMath::Vector4 C = SimpleMath::Vector4(0.211324865405187,
        // (3.0-sqrt(3.0))/6.0
        0.366025403784439,
        // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,
        // -1.0 + 2.0 * C.x
        0.024390243902439);
    // 1.0 / 41.0

// First corner (x0)
    SimpleMath::Vector2 i = SimpleMath::Vector2(floor(v.x + Dot(v, SimpleMath::Vector2(C.y,C.y))), floor(v.x + Dot(v, SimpleMath::Vector2(C.y, C.y))));
    SimpleMath::Vector2 x0 = v - SimpleMath::Vector2(i.x + Dot(i, SimpleMath::Vector2(C.x, C.x)), i.y + Dot(i, SimpleMath::Vector2(C.x, C.x)));

    // Other two corners (x1, x2)
    SimpleMath::Vector2 i1 = SimpleMath::Vector2(0,0);
    i1 = (x0.x > x0.y) ? SimpleMath::Vector2(1.0, 0.0) : SimpleMath::Vector2(0.0, 1.0);
    SimpleMath::Vector2 x1 = x0 + SimpleMath::Vector2(C.x,C.x) - i1;
    SimpleMath::Vector2 x2 = SimpleMath::Vector2(x0.x, x0.y) + SimpleMath::Vector2(C.z,C.z);

    // Do some permutations to avoid
    // truncation effects in permutation
    i = Mod289(i);
    SimpleMath::Vector3 p = Permute(
        Permute(SimpleMath::Vector3(i.y,i.y,i.y) + SimpleMath::Vector3(0.0, i1.y, 1.0))
        + SimpleMath::Vector3(i.x,i.x,i.x) + SimpleMath::Vector3(0.0, i1.x, 1.0));

    SimpleMath::Vector3 m = SimpleMath::Vector3 (Max(0.5 - (Dot(x0, x0)-Dot(x1, x1)-Dot(x2, x2) ), 0.0));

   

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    SimpleMath::Vector3  x = 2.0 * Frac3(p * SimpleMath::Vector3(C.w,C.w,C.w)) - SimpleMath::Vector3(1.0, 1.0, 1.0);
    SimpleMath::Vector3  h = Abs3(x) - SimpleMath::Vector3(0.5,0.5,0.5);
    SimpleMath::Vector3  ox = Floor3(x + SimpleMath::Vector3(0.5, 0.5, 0.5));
    SimpleMath::Vector3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= SimpleMath::Vector3(1.79284291400159, 1.79284291400159, 1.79284291400159 ) - SimpleMath::Vector3(0.85373472095314, 0.85373472095314, 0.85373472095314) * (a0 * a0 + h * h);

    // Compute final noise value at P
    SimpleMath::Vector3 g = SimpleMath::Vector3(0.0);
    g.x = a0.x * x0.x + h.x * x0.y;
    g.y = a0.y * x1.x + h.y * x1.y;
    g.z = a0.z * x2.x + h.z *  x2.y;
    return 130.0 * Dot3(m, g);
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
float NoiseFunctions::VoronoiTesselation(int x, int y, int scale) {
    
    std::vector<float> distances; 
    for (int i = 0; i < randomPoints.size();i++) {
        distances.push_back(Distance(randomPoints[i], SimpleMath::Vector2(x, y)));
    }
    std::sort(distances.begin(), distances.end());
    return distances[0]/scale;
}
float NoiseFunctions::Distance(SimpleMath::Vector2 a, SimpleMath::Vector2 b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    return sqrt((x * x) + (y * y));
}
float NoiseFunctions::RidgedMF(SimpleMath::Vector2 p) {
    perlinNoise.Initialize();
    float lacunarity = 2.0;
    float gain = 1;
    float offset = 0.9;

    float sum = 0.0;
    float freq = 1.0, amp = 1;
    float prev = 1.0;
    for (int i = 0; i < 5; i++) {
        float n = perlinNoise.Noise(p.x/10, p.y/10,1);
        sum += n * amp;
        sum += n * amp * prev;  // scale by previous octave
        prev = n;
        freq *= lacunarity;
        amp *= gain;
    }
    return sum;
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







