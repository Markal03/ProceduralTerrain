#pragma once
#include "DeviceResources.h"
#include "Light.h"
#include <SimpleMath.h>


class SkyDomeShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};
	struct ColorBufferType
	{
		XMFLOAT4 apexColor;
		XMFLOAT4 centerColor;
	};

public:
	SkyDomeShader();
	SkyDomeShader(const SkyDomeShader&);
	~SkyDomeShader();

	bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);		//Loads the Vert / pixel Shader pair
	bool SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection,
		XMFLOAT4 apexColor, XMFLOAT4 centerColor);
	void EnableShader(ID3D11DeviceContext* context);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMFLOAT4, XMFLOAT4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_colorBuffer;
};

