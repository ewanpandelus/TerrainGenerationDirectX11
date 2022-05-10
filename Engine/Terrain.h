#pragma once
#include "PerlinNoise.h"
#include "MeshShapes.h"
#include "NoiseFunctions.h"
#include "imgui.h"
using namespace DirectX;

class Terrain
{

private:
	PerlinNoise perlinNoise;
	NoiseFunctions noiseFunctions;
	bool m_inverseHeightmap = false;
	float m_scale = 0.1f;
	int m_terraceVal = 4;
	int m_seed = 0;
	float m_minHeight = 0;
	float m_maxHeight = 0;
	float m_flattenPercentage = 20;
	float m_bloomThreshold = 0;
	float m_cameraYPos = 0;
	std::vector<bool*> terrainTypes;
	std::vector<bool> prevTerrainTypes;


	float m_waterColour[3] = { 1,1,1 };
	float m_sandColour[3] = { 1,1,1 };
	float m_grassColour[3] = { 1,1,1 };
	float m_mellowSlopeColour[3] = { 1,1,1 };
	float m_steepSlopeColour[3] = { 1,1,1 };
	float m_snowColour[3] = { 1,1,1 };
	float m_terrainOffset = 0;

	float m_frequency, m_amplitude, m_lacunarity, m_persistance, m_offset;
	int m_octaves = 0;
	bool worley, m_ridge, m_fbm, m_terraced = false;
	bool m_overwritesColour = false;

	bool colourTerrain = false;
	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
	};
	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
		float prevY = 0, newY;
	};


public:
	Terrain();
	~Terrain();
	float GenerateValueWithActiveNoiseFunction(int i, int j, int terrainType);
	float Redistribution(float nx, float ny, float exp);
	float RidgeNoise(float x, float y);
	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device*);
	bool GenerateHeightMapLerped(ID3D11Device*);
	bool Update();
	float InverseLerp(float u, float v, float t);
	float Lerp(float u, float v, float t);
	void CalculateMaxMinNoiseHeight(float y, float* maxNoiseHeight, float* minNoiseHeight);
	void TerrainTypeTicked();
	void ManipulateTerrain(int x, int z, ID3D11Device* device, int up);
	void LerpTerrainHeight(ID3D11Device* device, float t);
	std::vector<SimpleMath::Vector3> randomPointsOnTerrain();


	float* SetWaterColour();
	std::vector<float> GetWaterColour();
	float* SetSandColour();
	std::vector<float> GetSandColour();
	float* SetGrassColour();
	std::vector<float> GetGrassColour();
	float* SetMellowSlopeColour();
	std::vector<float> GetMellowSlopeColour();
	float* SetSteepSlopeColour();
	std::vector<float> GetSteepSlopeColour();
	float* SetSnowColour();
	std::vector<float> GetSnowColour();


	float* GetPersistance();
	int* GetOctaves();
	float* GetLacunarity();
	float* GetOffset();
	float* GetFrequency();
	float GetMinHeight();
	float GetMaxHeight();
	bool* GetWorleyNoise();
	bool* GetTerraced();
	bool* GetFBMNoise();
	bool* GetInverseHeightMap();
	bool* GetRidgeNoise();
	bool* SetColourTerrain();
	bool* GetOverwritesColour();
	float GetCameraYPos();
	float* SetTerrainHeightPosition();
	int* SetSeed();
	int* SetTerraceVal();
	bool GetColourTerrain();
	std::vector<Triangle> GetTriangleArray();
	float* GetAmplitude();
	Box* GetBoxAtPosition(float x, float z);

private:
	bool CalculateNormals();
	void Shutdown();
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	void PopulateTriangles();
	void SetBoxBounds(Box _box);
	int FindActiveNoiseFunction();

	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;

	int m_vertexCount, m_indexCount;

	HeightMapType* m_heightMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
	std::vector<Triangle> triangleArray;
	std::vector<Box> boxArray;
};



