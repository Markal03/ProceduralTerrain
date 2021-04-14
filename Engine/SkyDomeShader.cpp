#include "pch.h"
#include "SkyDomeShader.h"


SkyDomeShader::SkyDomeShader()
{
}


SkyDomeShader::~SkyDomeShader()
{
}

bool SkyDomeShader::InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_BUFFER_DESC	colourBufferDesc;

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
	colourBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	colourBufferDesc.ByteWidth = sizeof(ColorBufferType);
	colourBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colourBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colourBufferDesc.MiscFlags = 0;
	colourBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&colourBufferDesc, NULL, &m_colorBuffer);

	return true;
}

bool SkyDomeShader::SetShaderParameters(
	ID3D11DeviceContext* context,
	DirectX::SimpleMath::Matrix* world,
	DirectX::SimpleMath::Matrix* view,
	DirectX::SimpleMath::Matrix* projection,
	XMFLOAT4 apexColor,
	XMFLOAT4 centerColor
)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	DirectX::SimpleMath::Matrix  tworld, tview, tproj;
	ColorBufferType* dataPtr2;

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

	context->Map(m_colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr2 = (ColorBufferType*)mappedResource.pData;

	dataPtr2->apexColor = apexColor;
	dataPtr2->centerColor = centerColor;

	context->Unmap(m_colorBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_colorBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS

	return false;
}

void SkyDomeShader::EnableShader(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
}
