#pragma once
#include "DeviceResources.h"
#include "Light.h"
#include <SimpleMath.h>

class WaterShader
{

	
	public:
		WaterShader();
		WaterShader(const WaterShader&);
		~WaterShader();

		bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);		//Loads the Vert / pixel Shader pair
		bool SetShaderParameters(
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
			float specularShininess);
		void EnableShader(ID3D11DeviceContext* context);
	private:
		struct MatrixBufferType
		{
			DirectX::SimpleMath::Matrix world;
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
			DirectX::SimpleMath::Matrix reflection;
		};

		struct CamNormBufferType
		{
			DirectX::SimpleMath::Vector3 cameraPosition;
			float padding1;
			DirectX::SimpleMath::Vector2 normalMapTiling;
			DirectX::SimpleMath::Vector2 padding2;
		};

		struct WaterBufferType
		{
			DirectX::SimpleMath::Vector4 refractionTint;
			DirectX::SimpleMath::Vector3 lightDirection;
			float waterTranslation;
			float reflectRefractScale;
			float specularShininess;
			DirectX::SimpleMath::Vector2 padding;
		};


	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
		ID3D11InputLayout* m_layout;
		ID3D11SamplerState* m_sampleState;
		ID3D11Buffer* m_matrixBuffer;
		ID3D11Buffer* m_camNormBuffer;
		ID3D11Buffer* m_waterBuffer;

};

