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
	float m_scale = 10;
	float m_minHeight = 0;
	float m_maxHeight = 0;
	float m_bloomThreshold = 0;

	std::vector<bool*> terrainTypes;
	std::vector<bool> prevTerrainTypes;


	float m_bottomTerrainColour[3] = { 1,1,1 };
	float m_secondTerrainColour[3] = { 1,1,1 };
	float m_thirdTerrainColour[3] = { 1,1,1 };
	float m_topTerrainColour[3] = { 1,1,1 };
	float m_terrainOffset = 0;

	float m_frequency, m_amplitude, m_lacunarity, m_persistance, m_octaves, m_offset;
	bool worley, ridge, fbm;


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
	};


public:
	Terrain();
	~Terrain();
	float GenerateValueWithActiveNoiseFunction(int i, int j, int terrainType, int scale);
	float Redistribution(float nx, float ny, float exp);
	float RidgeNoise(float x, float y);
	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device*);
	bool Update();
	float InverseLerp(float u, float v, float t);
	float Lerp(float u, float v, float t);

	
	void TerrainTypeTicked();
	float* SetBottomTerrainColorImGUI();
	std::vector<float> GetBottomTerrainColour();
	float* SetSecondTerrainColourImGUI();
	std::vector<float> GetSecondTerrainColour();
	float* SetThirdTerrainColorImGUI();
	std::vector<float> GetThirdTerrainColour();
	float* SetTopTerrainColorImGUI();
	std::vector<float> GetTopTerrainColour();
	float* GetPersistance();
	float* GetOctaves();
	float* GetLacunarity();
	float* GetOffset();
	float* GetFrequency();
	float GetMinHeight();
	float GetMaxHeight();
	bool* GetWorleyNoise();
	bool* GetFBMNoise();
	bool* GetRidgeNoise();
	bool* SetColourTerrain();

	float* SetTerrainHeightPosition();
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



