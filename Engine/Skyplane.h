////////////////////////////////////////////////////////////////////////////////
// Filename: skyplaneclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SKYPLANE_H_
#define _SKYPLANE_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <SimpleMath.h>
#include "RenderTexture.h"
#include "Texture.h"
using namespace DirectX::SimpleMath;

class Skyplane
{
private:
	struct SkyplaneType
	{
		float x, y, z;
		float tu, tv;
	};

	struct VertexType
	{
		Vector3 position;
		Vector2 texture;
	};

public:
	Skyplane();
	Skyplane(const Skyplane&);
	~Skyplane();

	bool Initialize(ID3D11Device*, WCHAR*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	void Frame();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetCloudTexture();
	ID3D11ShaderResourceView* GetPerturbTexture();
	float GetScale();
	float GetBrightness();
	float GetTranslation();

private:
	bool InitializeSkyPlane(int, float, float, float, int);
	void ShutdownSkyPlane();

	bool InitializeBuffers(ID3D11Device*, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, WCHAR*, WCHAR*);
	void ReleaseTextures();

private:
	SkyplaneType* m_skyPlane;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;

	Texture* m_CloudTexture, * m_PerturbTexture;
	float m_scale, m_brightness, m_translation;
};

#endif