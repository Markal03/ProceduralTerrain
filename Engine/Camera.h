#pragma once
using namespace DirectX::SimpleMath;
class Camera
{
public:
	Camera();
	~Camera();

	void							Update();
	Matrix							getCameraMatrix();
	void							setPosition(Vector3 newPosition);
	Vector3							getPosition();
	Vector3							getForward();
	void							setRotation(Vector3 newRotation);
	Vector3							getRotation();
	float							getMoveSpeed();
	float							getRotationSpeed();
	void							RenderReflection(float);
	Matrix							GetReflectionViewMatrix();
	Vector3							getUp();
	Vector3							getRight();
private:
	Matrix							m_cameraMatrix, m_viewMatrix, m_reflectionViewMatrix;			//camera matrix to be passed out and used to set camera position and angle for wrestling
	Vector3							m_lookat;
	Vector3							m_position;
	Vector3							m_forward;
	Vector3							m_right;
	Vector3							m_up;
	Vector3							m_orientation;			//vector storing pitch yaw and roll. 

	float							m_movespeed;	
	float							m_camRotRate;

};

