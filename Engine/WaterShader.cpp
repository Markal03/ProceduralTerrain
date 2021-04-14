#include "pch.h"
#include "WaterShader.h"


WaterShader::WaterShader()
{
}


WaterShader::~WaterShader()
{
}

bool WaterShader::InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_SAMPLER_DESC	samplerDesc;
	D3D11_BUFFER_DESC	camNormBufferDesc;
	D3D11_BUFFER_DESC	waterBufferDesc;

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

	camNormBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	camNormBufferDesc.ByteWidth = sizeof(CamNormBufferType);
	camNormBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camNormBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camNormBufferDesc.MiscFlags = 0;
	camNormBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&camNormBufferDesc, NULL, &m_camNormBuffer);


	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&waterBufferDesc, NULL, &m_waterBuffer);

	return true;
}

bool WaterShader::SetShaderParameters(
	ID3D11DeviceContext* context,
	DirectX::SimpleMath::Matrix* world,
	DirectX::SimpleMath::Matrix* view,
	DirectX::SimpleMath::Matrix* projection,
	DirectX::SimpleMath::Matrix* reflection,
	ID3D11ShaderResourceView* refractionTexture,
	ID3D11ShaderResourceView* reflectionTexture,
	ID3D11ShaderResourceView* normalTexture,
	DirectX::SimpleMath::Vector3 cameraPosition,
	DirectX::SimpleMath::Vector2 normalMapTiling,
	float waterTranslation,
	float reflectRefractScale,
	DirectX::SimpleMath::Vector4 refractionTint,
	Light* sceneLight1,
	float specularShininess
)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	CamNormBufferType* dataPtr2;
	WaterBufferType* dataPtr3;
	DirectX::SimpleMath::Matrix  tworld, tview, tproj, trefl;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	trefl = reflection->Transpose();

	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->reflection = trefl;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS

	context->Map(m_camNormBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (CamNormBufferType*)mappedResource.pData;

	// Copy the data into the constant buffer.
	dataPtr2->cameraPosition = cameraPosition;
	dataPtr2->padding1 = 0.0f;
	dataPtr2->normalMapTiling = normalMapTiling;
	dataPtr2->padding2 = DirectX::SimpleMath::Vector2(0.0f, 0.0f);

	// Unlock the constant buffer.
	context->Unmap(m_camNormBuffer, 0);
	context->VSSetConstantBuffers(1, 1, &m_camNormBuffer);

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &refractionTexture);
	context->PSSetShaderResources(1, 1, &reflectionTexture);
	context->PSSetShaderResources(2, 1, &normalTexture);


	context->Map(m_waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr3 = (WaterBufferType*)mappedResource.pData;
	
	dataPtr3->waterTranslation = waterTranslation;
	dataPtr3->reflectRefractScale = reflectRefractScale;
	dataPtr3->refractionTint = refractionTint;
	dataPtr3->lightDirection = sceneLight1->getDirection();
	dataPtr3->specularShininess = specularShininess;
	dataPtr3->padding = SimpleMath::Vector2(0.0f, 0.0f);

	context->Unmap(m_waterBuffer, 0);

	context->PSSetConstantBuffers(0, 1, &m_waterBuffer);

	return false;
}

void WaterShader::EnableShader(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), NULL, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), NULL, 0);				//turn on pixel shader
	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &m_sampleState);

}
