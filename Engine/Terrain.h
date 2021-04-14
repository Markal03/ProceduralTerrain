#pragma once

using namespace DirectX;
#include "RenderTexture.h"

const int TEXTURE_REPEAT = 32;

class Terrain
{
private:
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

	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device*);
	float GenerateRandomNumberBetween(float _min, float _max);
	void CalculateTextureCoordinates();
	float Noise(int x);
	float CosInterpolate(float v1, float v2, float a);
	bool Flatten(ID3D11Device*);
	bool GenerateFaulting(ID3D11Device*);
	bool GeneratePerlinNoise(ID3D11Device*, int seed, float noiseSize, float persistence, int octaves);
	float PerlinNoise2D(int seed, float persistence, int octave, float x, float y);
	float GetNeighboursAverageHeight(int j, int i);
	bool Smooth(ID3D11Device*);
	bool Update();
	//bool RayTriangleIntersect(const Vector3 orig, const Vector3 dir, const Vector3 v0, const Vector3 v1, const Vector3 v2);
	float* GetWavelength();
	float* GetAmplitude();

	float* GetNoiseSize();
	float* GetPersistence();
	int* GetOctaves();
	int* GetSeed();
	int* GetIndexCount();

	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();
private:
	bool CalculateNormals();
	void Shutdown();
	void ShutdownBuffers();
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	

private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer * m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	float m_frequency, m_amplitude, m_wavelength;
	float m_noiseSize, m_persistence;
	int m_seed, m_octaves;
	HeightMapType* m_heightMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
};

