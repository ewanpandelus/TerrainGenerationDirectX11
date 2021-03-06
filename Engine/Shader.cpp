#include "pch.h"
#include "Shader.h"



Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_SAMPLER_DESC	samplerDesc;
	D3D11_BUFFER_DESC	lightBufferDesc;
	D3D11_BUFFER_DESC   terrainBufferDesc;
	D3D11_BUFFER_DESC   terrainExtraBufferDesc;
	D3D11_BUFFER_DESC   otherBufferDesc;
	//LOAD SHADER:	VERTEX
	auto vertexShaderBuffer = DX::ReadData(vsFilename);
	HRESULT result = device->CreateVertexShader(vertexShaderBuffer.data(), vertexShaderBuffer.size(), NULL, &m_vertexShader);
	if (result != S_OK)
	{
		//if loading failed.  
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the MeshClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer.data(), vertexShaderBuffer.size(), &m_layout);


	//LOAD SHADER:	PIXEL
	auto pixelShaderBuffer = DX::ReadData(psFilename);
	result = device->CreatePixelShader(pixelShaderBuffer.data(), pixelShaderBuffer.size(), NULL, &m_pixelShader);
	if (result != S_OK)
	{
		//if loading failed. 
		return false;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);


	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

	terrainBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainBufferDesc.ByteWidth = sizeof(LightBufferType);
	terrainBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainBufferDesc.MiscFlags = 0;
	terrainBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&terrainBufferDesc, NULL, &m_terrainColourBuffer);

	terrainExtraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainExtraBufferDesc.ByteWidth = sizeof(LightBufferType);
	terrainExtraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainExtraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainExtraBufferDesc.MiscFlags = 0;
	terrainExtraBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&terrainExtraBufferDesc, NULL, &m_terrainExtraVariablesBuffer);

	otherBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	otherBufferDesc.ByteWidth = sizeof(LightBufferType);
	otherBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	otherBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	otherBufferDesc.MiscFlags = 0;
	otherBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&otherBufferDesc, NULL, &m_otherVariablesBufferType);


	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	device->CreateSamplerState(&samplerDesc, &m_sampleState);

	return true;
}

bool Shader::SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, Light* sceneLight1, ID3D11ShaderResourceView* texture1)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;

	DirectX::SimpleMath::Matrix  tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS

	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = sceneLight1->getAmbientColour();
	lightPtr->diffuse = sceneLight1->getDiffuseColour();
	lightPtr->position = sceneLight1->getPosition();
	lightPtr->padding = 0.0f;
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture1);

	return false;
}

bool Shader::SetShaderParametersTerrain(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, Light* sceneLight1,
	ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3,
	ID3D11ShaderResourceView* texture4, ID3D11ShaderResourceView* texture5, ID3D11ShaderResourceView* texture6, float time, Terrain terrain, int won)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	TimeBufferType* timePtr;
	TerrainColourBufferType* terrainColourPtr;
	TerrainExtraVariablesBufferType* terrainExtraPtr;
	OtherVariablesBufferType* otherPtr;


	DirectX::SimpleMath::Matrix  tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->time = time;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS



	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = sceneLight1->getAmbientColour();
	lightPtr->diffuse = sceneLight1->getDiffuseColour();
	lightPtr->position = sceneLight1->getPosition();
	lightPtr->padding = time;



	//	context->PSSetConstantBuffers(1, 1, &timePtr);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS


	context->Map(m_terrainColourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	terrainColourPtr = (TerrainColourBufferType*)mappedResource.pData;
	std::vector<float> sandColour = terrain.GetSandColour();
	std::vector<float> grassColour = terrain.GetGrassColour();
	std::vector<float> mellowSlopeColour = terrain.GetMellowSlopeColour();
	std::vector<float> snowColour = terrain.GetSnowColour();

	if (terrain.GetColourTerrain()) {

		terrainColourPtr->sandColour = SimpleMath::Vector4(sandColour[0], sandColour[1], sandColour[2], 1);
		terrainColourPtr->grassColour = SimpleMath::Vector4(grassColour[0], grassColour[1], grassColour[2], 1);
		terrainColourPtr->mellowSlopeColour = SimpleMath::Vector4(mellowSlopeColour[0], mellowSlopeColour[1], mellowSlopeColour[2], 1);

	}
	else {
		terrainColourPtr->sandColour = SimpleMath::Vector4::One;
		terrainColourPtr->grassColour = SimpleMath::Vector4::One;
		terrainColourPtr->mellowSlopeColour = SimpleMath::Vector4::One;
		terrainColourPtr->excess1 = 0;
	}
	context->Unmap(m_terrainColourBuffer, 0);
	context->PSSetConstantBuffers(1, 1, &m_terrainColourBuffer);



	context->Map(m_terrainExtraVariablesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	terrainExtraPtr = (TerrainExtraVariablesBufferType*)mappedResource.pData;
	std::vector<float> waterColour = terrain.GetWaterColour();
	std::vector<float> steepSlopeColour = terrain.GetSteepSlopeColour();
	bool overwrite = *terrain.GetOverwritesColour();
	terrainExtraPtr->overwritesColour = SimpleMath::Vector4(overwrite, overwrite, overwrite, overwrite);
	if (terrain.GetColourTerrain()) {

		terrainExtraPtr->waterColour = SimpleMath::Vector4(waterColour[0], waterColour[1], waterColour[2], 1);
		terrainExtraPtr->steepSlopeColour = SimpleMath::Vector4(steepSlopeColour[0], steepSlopeColour[1], steepSlopeColour[2], 1);

	}
	else {
		terrainExtraPtr->waterColour = SimpleMath::Vector4::One;
		terrainExtraPtr->steepSlopeColour = SimpleMath::Vector4::One;
	}
	terrainExtraPtr->excess = 0;

	context->Unmap(m_terrainExtraVariablesBuffer, 0);
	context->PSSetConstantBuffers(2, 1, &m_terrainExtraVariablesBuffer);

	context->Map(m_otherVariablesBufferType, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	otherPtr = (OtherVariablesBufferType*)mappedResource.pData;
	otherPtr->snowColour = SimpleMath::Vector4(snowColour[0], snowColour[1], snowColour[2], 1);

	float w = won ? 1 : 0;
	otherPtr->winGame = w;
	otherPtr->pad1 = 0;
	otherPtr->pad2 = 0;

	context->Unmap(m_otherVariablesBufferType, 0);
	context->PSSetConstantBuffers(3, 1, &m_otherVariablesBufferType);
	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture1);
	context->PSSetShaderResources(1, 1, &texture2);
	context->PSSetShaderResources(2, 1, &texture3);
	context->PSSetShaderResources(3, 1, &texture4);
	context->PSSetShaderResources(4, 1, &texture5);
	context->PSSetShaderResources(5, 1, &texture6);





	return false;
}

void Shader::EnableShader(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &m_sampleState);

}
