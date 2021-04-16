#pragma once

#include "DeviceResources.h"
#include "Light.h"
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

class SkyplaneShader
{
public:
	SkyplaneShader();
	~SkyplaneShader();

	//we could extend this to load in only a vertex shader, only a pixel shader etc.  or specialised init for Geometry or domain shader. 
	//All the methods here simply create new versions corresponding to your needs
	bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);		//Loads the Vert / pixel Shader pair
	bool SetShaderParameters(
		ID3D11DeviceContext* context,
		Matrix* world,
		Matrix* view,
		Matrix* projection,
		ID3D11ShaderResourceView* colorTexture,
		ID3D11ShaderResourceView* normalTexture,
		float translation,
		float scale,
		float brightness);
	void EnableShader(ID3D11DeviceContext* context);

private:
	//standard matrix buffer supplied to all shaders
	struct MatrixBufferType
	{
		Matrix world;
		Matrix view;
		Matrix projection;
	};

	struct SkyBufferType
	{
		float translation;
		float scale;
		float brightness;
		float padding;
	};

	//buffer for information of a single light
	struct LightBufferType
	{
		Vector4 lightDiffuseColor;
		Vector3 lightDirection;
		float colorTextureBrightness;
	};

	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_skyBuffer;
};