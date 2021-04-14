#pragma once
#include "RenderTexture.h"
class Water
{
private:
	struct VertexType
	{
		DirectX::SimpleMath::Vector3		position;
		DirectX::SimpleMath::Vector2		texture;
	};

public:
	Water();
	Water(const Water&);
	~Water();

	bool Initialize(ID3D11Device*, WCHAR*, float, float);
	void Shutdown();
	void Frame();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	float GetWaterHeight();
	DirectX::SimpleMath::Vector2 GetNormalMapTiling();
	float GetWaterTranslation();
	float GetReflectRefractScale();
	DirectX::SimpleMath::Vector4 GetRefractionTint();
	float GetSpecularShininess();

private:
	bool InitializeBuffers(ID3D11Device*, float);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	float m_waterHeight;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;
	RenderTexture* m_Texture;
	DirectX::SimpleMath::Vector2 m_normalMapTiling;
	float m_waterTranslation;
	float m_reflectRefractScale;
	DirectX::SimpleMath::Vector4 m_refractionTint;
	float m_specularShininess;
};


