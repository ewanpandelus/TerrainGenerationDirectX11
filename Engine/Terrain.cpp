#include "pch.h"
#include "Terrain.h"
#include  <cmath>
#include <iostream>




Terrain::Terrain()
{
	m_terrainGeneratedToggle = false;
	noiseFunctions.Setup();
	worley = false;
	m_ridge = false;
	m_fbm = false;
	m_terraced = false;
	perlinNoise.Initialize();

}


Terrain::~Terrain()
{
}

void Terrain::TerrainTypeTicked() {
	int index = -1;
	for (int i = 0; i < terrainTypes.size();i++) {
		if (prevTerrainTypes[i] != *terrainTypes[i] && *terrainTypes[i] == true)
		{
			index = i;
			break;
		}
	}
	if (index == -1) return;

	for (int i = 0; i < terrainTypes.size();i++) {
		if (i != index) {
			*terrainTypes[i] = false;
			prevTerrainTypes[i] = false;
		}
	}
	prevTerrainTypes[index] = true;

}

bool Terrain::Initialize(ID3D11Device* device, int terrainWidth, int terrainHeight)
{


	int index;
	float height = 0.0;
	bool result;

	terrainTypes.push_back(&worley);
	terrainTypes.push_back(&m_ridge);
	terrainTypes.push_back(&m_fbm);
	prevTerrainTypes.push_back(worley);
	prevTerrainTypes.push_back(m_ridge);
	prevTerrainTypes.push_back(m_fbm);
	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	m_frequency = 1.f;
	m_amplitude = 3.0;
	m_lacunarity = 0.5;
	m_octaves = 3;
	m_offset = 0.01;
	m_persistance = 1.25f;
	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	//this is how we calculate the texture coordinates first calculate the step size there will be between vertices. 
	float textureCoordinatesStep = 5.0f / m_terrainWidth;  //tile 5 times across the terrain. 
	// Initialise the data in the height map (flat).
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			//and use this step to calculate the texture coordinates for this point on the terrain.
			m_heightMap[index].u = (float)i * textureCoordinatesStep;
			m_heightMap[index].v = (float)j * textureCoordinatesStep;

		}
	}

	//even though we are generating a flat terrain, we still need to normalise it. 
	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}


	return true;
}

void Terrain::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}
std::vector<SimpleMath::Vector3> Terrain::randomPointsOnTerrain() {
	std::vector<SimpleMath::Vector3> randPoints;
	for (int i = 0; i < 8;i++)
	{
		int index = rand() % ((m_terrainHeight - 1) * (m_terrainWidth - 1));
		randPoints.push_back(SimpleMath::Vector3(m_heightMap[index].x * 0.5f, m_heightMap[index].y * 0.5f, m_heightMap[index].z * 0.5f));
	}
	return randPoints;
}
bool Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	DirectX::SimpleMath::Vector3* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new DirectX::SimpleMath::Vector3[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
	if (!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i + 1);
			index3 = ((j + 1) * m_terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;

			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;

			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

void Terrain::Shutdown()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

bool Terrain::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index, i, j;
	int index1, index2, index3, index4; //geometric indices. 

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;
	int counter = 0;
	triangleArray.clear();
	boxArray.clear();
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{

			if (counter % 2 == 0) {

				index1 = (m_terrainHeight * j) + i;             //Bottom left
				index2 = (m_terrainHeight * j) + (i + 1);      // Bottom right
				index3 = (m_terrainHeight * (j + 1)) + i;       //Upper left 
				index4 = (m_terrainHeight * (j + 1)) + (i + 1); //Upper right 
			}
			else {
				index1 = (m_terrainHeight * (j + 1)) + i;       //Upper left 
				index3 = (m_terrainHeight * (j + 1)) + (i + 1); //Upper right 
				index2 = (m_terrainHeight * j) + i;             //Bottom left
				index4 = (m_terrainHeight * j) + (i + 1);      // Bottom right

			}
			counter++;
			//	Box box;
			Triangle triangle;


			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].u, m_heightMap[index3].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[0] = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);

			// Upper right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].u, m_heightMap[index4].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[1] = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);


			// Bottom left.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].u, m_heightMap[index1].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[2] = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			triangleArray.push_back(triangle);


			//box.triangles[0] = triangle;
			// Bottom left.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].u, m_heightMap[index1].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[0] = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);

			// Upper right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].u, m_heightMap[index4].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[1] = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);

			// Bottom right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].u, m_heightMap[index2].v);
			indices[index] = index;
			index++;

			triangle.trianglePositions[2] = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			triangleArray.push_back(triangle);

			//box.triangles[1] = triangle;

			//SetBoxBounds(box);

		}

	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}
void Terrain::ManipulateTerrain(int x, int z, ID3D11Device* device, int up) {
	bool result;

	if (x <= 0 || x == m_terrainWidth || z == m_terrainHeight || z <= 0) return;
	int index = ((m_terrainHeight * z) + x) * 2;
	if (index > 20000) return;
	m_heightMap[index].y += 1 * up;
	m_heightMap[(index)+1].y += 1 * up;
	m_heightMap[(index)+1 + m_terrainHeight].y += 1 * up;
	m_heightMap[(index)+m_terrainHeight].y += 1 * up;
	m_heightMap[(index)-m_terrainHeight].y += 1 * up;
	m_heightMap[(index)-1 + m_terrainHeight].y += 1 * up;
	m_heightMap[(index)-1].y += 1 * up;
	m_heightMap[(index)-1 - m_terrainHeight].y += 1 * up;
	m_heightMap[(index)+1 - m_terrainHeight].y += 1 * up;
	CalculateNormals();
	InitializeBuffers(device);

}
void Terrain::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
void Terrain::CalculateMaxMinNoiseHeight(float y, float* minNoiseHeight, float* maxNoiseHeight) {
	if (y > * maxNoiseHeight) {
		*maxNoiseHeight = y;
	}
	else if (y < *minNoiseHeight) {
		*minNoiseHeight = y;
	}
}
int Terrain::FindActiveNoiseFunction() {
	int index = -1;
	for (int i = 0; i < prevTerrainTypes.size();i++) {
		if (prevTerrainTypes[i] == true)
		{
			return i;
		}
	}
	return -1;
}
float Terrain::GenerateValueWithActiveNoiseFunction(int i, int j, int terrainType) {

	if (m_terraced) terrainType = 3;
	switch (terrainType) {
	case 1:
		return noiseFunctions.RidgeNoise(i, j, m_scale, m_frequency);
		break;
	case 2:
		return noiseFunctions.PerlinNoise(i, j, m_scale, m_frequency);
	case 3:
		return Redistribution(i, j, m_terraceVal);
		break;

	default:
		return 0;
		break;
	}
}
void Terrain::LerpTerrainHeight(ID3D11Device* device, float t) {
	int index;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].y = Lerp(m_heightMap[index].prevY, m_heightMap[index].newY, t);
			if (t >= 0.99) {
				m_heightMap[index].prevY = m_heightMap[index].newY;
			}
		}
	}

	CalculateNormals();
	InitializeBuffers(device);
}
bool Terrain::GenerateHeightMap(ID3D11Device* device)
{
	bool result;

	float initialAmp = m_amplitude;
	float initialFrequency = m_frequency;
	int index;
	float scale = 10;
	int terrainType = FindActiveNoiseFunction();
	if (terrainType == -1) return false;

	//we want a wavelength of 1 to be a single wave over the whole terrain.  A single wave is 2 pi which is about 6.283

	//loop through the terrain and set the hieghts how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.

	float maxNoiseHeight = -1000;
	float minNoiseHeight = 1000;

	noiseFunctions.PopulateRandomPoints(m_terrainWidth, m_terrainHeight, 32, 1);
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].x = (float)i;
			m_heightMap[index].z = (float)j;
			float noiseHeight = 0;
			if (terrainType == 0)
			{
				m_heightMap[index].y = (noiseFunctions.VoronoiTesselation(i + m_seed * m_frequency, j * m_frequency) * m_amplitude) + m_terrainOffset;
				CalculateMaxMinNoiseHeight(m_heightMap[index].y, &minNoiseHeight, &maxNoiseHeight);
				continue;
			}


			m_amplitude = initialAmp;
			m_frequency = initialFrequency;
			for (int octave = 0; octave < m_octaves; octave++) {
				float perlinValue = GenerateValueWithActiveNoiseFunction(i + m_seed, j + m_seed, terrainType);
				noiseHeight += perlinValue * m_amplitude;
				m_amplitude *= m_persistance;
				m_frequency *= m_lacunarity;
			}

			CalculateMaxMinNoiseHeight(noiseHeight, &minNoiseHeight, &maxNoiseHeight);
			m_heightMap[index].y = noiseHeight + (!m_inverseHeightmap * m_terrainOffset);

		}

	}
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			float inverseLerp = InverseLerp(minNoiseHeight, maxNoiseHeight, m_heightMap[index].y);
			float inverted = Lerp(minNoiseHeight, maxNoiseHeight, 1 - inverseLerp);
			if (m_inverseHeightmap) {
				m_heightMap[index].y = inverted + m_terrainOffset;
			}
			if ((m_inverseHeightmap && inverseLerp > (1 - (m_flattenPercentage / 100)) || (!m_inverseHeightmap && inverseLerp < (m_flattenPercentage / 100)))) {
				m_heightMap[index].y = Lerp(minNoiseHeight, maxNoiseHeight, 0.1);
			}
		}
	}
	m_cameraYPos = ((maxNoiseHeight + m_terrainOffset));
	m_amplitude = initialAmp;
	m_frequency = initialFrequency;
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}
float Terrain::InverseLerp(float u, float v, float value)
{
	return (value - u) / (v - u);
}
float Terrain::Lerp(float u, float v, float t)
{
	return u + (t * (v - u));
}

float Terrain::Redistribution(float x, float y, float exponent) {
	float e0 = 1 * perlinNoise.Noise(1 * x * m_scale * m_frequency, 1 * y * m_scale * m_frequency, 1);
	float e1 = 0.5 * perlinNoise.Noise(2 * x * m_scale * m_frequency, 2 * y * m_scale * m_frequency, 1) * e0;
	float e2 = 0.25 * perlinNoise.Noise(4 * x * m_scale * m_frequency, 4 * y * m_scale * m_frequency, 1) * (e0 + e1);
	float e = (e0 + e1 + e2) / (1 + 0.5 + 0.25);
	return round(e * exponent) / exponent;
}

void Terrain::PopulateTriangles() {

}
bool Terrain::Update()
{
	return true;
}
std::vector<float> Terrain::GetWaterColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_waterColour[0]);
	colorVals.push_back(m_waterColour[1]);
	colorVals.push_back(m_waterColour[2]);

	return colorVals;
}
float* Terrain::SetWaterColour()
{
	return &m_waterColour[0];
}
std::vector<float> Terrain::GetSandColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_sandColour[0]);
	colorVals.push_back(m_sandColour[1]);
	colorVals.push_back(m_sandColour[2]);

	return colorVals;
}
float* Terrain::SetSandColour()
{
	return &m_sandColour[0];
}

std::vector<float> Terrain::GetGrassColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_grassColour[0]);
	colorVals.push_back(m_grassColour[1]);
	colorVals.push_back(m_grassColour[2]);

	return colorVals;
}
float* Terrain::SetGrassColour()
{
	return &m_grassColour[0];
}
std::vector<float> Terrain::GetMellowSlopeColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_mellowSlopeColour[0]);
	colorVals.push_back(m_mellowSlopeColour[1]);
	colorVals.push_back(m_mellowSlopeColour[2]);

	return colorVals;
}
float* Terrain::SetMellowSlopeColour()
{
	return &m_mellowSlopeColour[0];
}
std::vector<float> Terrain::GetSteepSlopeColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_steepSlopeColour[0]);
	colorVals.push_back(m_steepSlopeColour[1]);
	colorVals.push_back(m_steepSlopeColour[2]);

	return colorVals;
}
float* Terrain::SetSteepSlopeColour()
{
	return &m_steepSlopeColour[0];
}
std::vector<float> Terrain::GetSnowColour()
{
	std::vector<float> colorVals;
	colorVals.push_back(m_snowColour[0]);
	colorVals.push_back(m_snowColour[1]);
	colorVals.push_back(m_snowColour[2]);

	return colorVals;
}
float* Terrain::SetSnowColour()
{
	return &m_snowColour[0];
}

float* Terrain::SetTerrainHeightPosition() {

	return &m_terrainOffset;
}


float Terrain::GetMinHeight()
{
	return m_minHeight;
}
float Terrain::GetMaxHeight()
{
	return m_maxHeight;

}
bool* Terrain::GetInverseHeightMap()
{
	return &m_inverseHeightmap;

}
bool* Terrain::GetOverwritesColour()
{
	return &m_overwritesColour;

}
bool* Terrain::GetTerraced()
{
	return &m_terraced;

}
int* Terrain::GetOctaves()
{
	return &m_octaves;

}
bool* Terrain::GetWorleyNoise()
{
	return &worley;

}
bool* Terrain::GetFBMNoise()
{
	return &m_fbm;

}

bool* Terrain::GetRidgeNoise() {

	return &m_ridge;
}
bool* Terrain::SetColourTerrain()
{
	return &colourTerrain;
}
bool Terrain::GetColourTerrain()
{
	return colourTerrain;
}
float* Terrain::GetOffset()
{

	return &m_offset;

}
float* Terrain::GetLacunarity()
{
	return &m_lacunarity;

}
float* Terrain::GetPersistance()
{
	return &m_persistance;
}

float* Terrain::GetAmplitude()
{
	return &m_amplitude;
}
int* Terrain::SetSeed()
{
	return &m_seed;
}
int* Terrain::SetTerraceVal()
{
	return &m_terraceVal;
}

float* Terrain::GetFrequency()
{
	return &m_frequency;
}
float Terrain::GetCameraYPos() {
	return m_cameraYPos;
}
std::vector<Triangle> Terrain::GetTriangleArray()
{
	return triangleArray;
}
void Terrain::SetBoxBounds(Box _box) {
	int _xMin = INT8_MAX;
	int _zMin = INT8_MAX;
	int _xMax = INT8_MIN;
	int _zMax = INT8_MIN;
	for (int triangleCount = 0;triangleCount < 2;triangleCount++) {
		for each (Triangle triangle in _box.triangles) {
			for each (DirectX::SimpleMath::Vector3 pos in triangle.trianglePositions) {
				if (pos.x < _xMin) {
					_xMin = pos.x;
				}
				if (pos.x > _xMax) {
					_xMax = pos.x;
				}
				if (pos.z < _zMin) {
					_zMin = pos.z;
				}
				if (pos.z > _zMax) {
					_zMax = pos.z;
				}
			}
		}
	}
	_box.xMax = _xMax;
	_box.xMin = _xMin;
	_box.zMax = _zMax;
	_box.zMin = _zMin;
	boxArray.push_back(_box);

}
Box* Terrain::GetBoxAtPosition(float x, float z) {
	for each (Box box in boxArray) {
		if (x > box.xMin && x<box.xMax && z> box.zMin && z < box.zMax) {
			return new Box(box);
		}
	}
	return NULL;

}
bool Terrain::GenerateHeightMapLerped(ID3D11Device* device)
{
	bool result;

	float initialAmp = m_amplitude;
	float initialFrequency = m_frequency;
	int index;
	float scale = 10;
	int terrainType = FindActiveNoiseFunction();
	if (terrainType == -1) return false;

	//we want a wavelength of 1 to be a single wave over the whole terrain.  A single wave is 2 pi which is about 6.283

	//loop through the terrain and set the hieghts how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.

	float maxNoiseHeight = -1000;
	float minNoiseHeight = 1000;

	noiseFunctions.PopulateRandomPoints(m_terrainWidth, m_terrainHeight, 32, 1);
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].x = (float)i;
			m_heightMap[index].z = (float)j;
			float noiseHeight = 0;
			if (terrainType == 0)
			{
				m_heightMap[index].newY = (noiseFunctions.VoronoiTesselation(i + m_seed * m_frequency, j * m_frequency) * m_amplitude) + m_terrainOffset;
				CalculateMaxMinNoiseHeight(m_heightMap[index].newY, &minNoiseHeight, &maxNoiseHeight);
				continue;
			}


			m_amplitude = initialAmp;
			m_frequency = initialFrequency;
			for (int octave = 0; octave < m_octaves; octave++) {
				float perlinValue = GenerateValueWithActiveNoiseFunction(i + m_seed, j + m_seed, terrainType);
				noiseHeight += perlinValue * m_amplitude;
				m_amplitude *= m_persistance;
				m_frequency *= m_lacunarity;
			}

			CalculateMaxMinNoiseHeight(noiseHeight, &minNoiseHeight, &maxNoiseHeight);
			m_heightMap[index].newY = noiseHeight + (!m_inverseHeightmap * m_terrainOffset);

		}

	}
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			float inverseLerp = InverseLerp(minNoiseHeight, maxNoiseHeight, m_heightMap[index].newY);
			float inverted = Lerp(minNoiseHeight, maxNoiseHeight, 1 - inverseLerp);
			if (m_inverseHeightmap) {
				m_heightMap[index].newY = inverted + m_terrainOffset;
			}
			if ((m_inverseHeightmap && inverseLerp > (1 - (m_flattenPercentage / 100)) || (!m_inverseHeightmap && inverseLerp < (m_flattenPercentage / 100)))) {
				m_heightMap[index].newY = Lerp(minNoiseHeight, maxNoiseHeight, 0.1);
			}
		}
	}
	m_cameraYPos = ((maxNoiseHeight + m_terrainOffset));
	m_amplitude = initialAmp;
	m_frequency = initialFrequency;
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}