#pragma once
#include "PerlinNoise.h"
#include "MeshShapes.h"
using namespace DirectX;

class Ocean
{
public:

private:
	PerlinNoise perlinNoise;
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
	Ocean();
	~Ocean();
	float m_scale = 10;
	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device*);
	bool Update();


private:
	bool CalculateNormals();
	void Shutdown();
	void ShutdownBuffers();
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);




private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;

	int m_vertexCount, m_indexCount;
	float m_frequency, m_amplitude, m_lacunarity, m_persistance, m_octaves, m_offset;
	HeightMapType* m_heightMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
	std::vector<Triangle> triangleArray;
	std::vector<Box> boxArray;
};



