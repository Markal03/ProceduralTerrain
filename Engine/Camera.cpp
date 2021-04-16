#include "pch.h"
#include "Camera.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
//camera for our app simple directX application. While it performs some basic functionality its incomplete.
//

Camera::Camera()
{
	m_orientation.x = 0.0f;		//rotation around x - pitch
	m_orientation.y = 0.0f;		//rotation around y - yaw
	m_orientation.z = 0.0f;		//rotation around z - roll	//we tend to not use roll a lot in first person

	m_position.x = 0.0f;		//camera position in space.
	m_position.y = 0.0f;
	m_position.z = 0.0f;

}



Camera::~Camera()
{
}

void Camera::Update()
{

	Vector3 up, position, lookAt;
	float yaw, pitch, roll;
	Matrix rotationMatrix;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	position.x = m_position.x;
	position.y = m_position.y;
	position.z = m_position.z;

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = m_position.x * 0.0174532925f;
	yaw = m_position.y * 0.0174532925f;
	roll = m_position.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix =  XMMatrixRotationRollPitchYaw(roll, pitch, yaw);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);


}

void Camera::RenderReflection(float height)
{
	Vector3 up, position, lookAt;
	float yaw, pitch, roll;
	Matrix rotationMatrix;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	position.x = m_position.x;
	position.y = -m_position.y + (height * 2.0f);
	position.z = m_position.z;

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = m_position.x * 0.0174532925f; // 0.0174532925f = PI / 180
	yaw = m_position.y * 0.0174532925f;
	roll = m_position.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(roll, pitch, yaw);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	m_reflectionViewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}


Matrix Camera::getCameraMatrix()
{
	return m_viewMatrix;
}

Matrix Camera::GetReflectionViewMatrix()
{
	return m_reflectionViewMatrix;
}

void Camera::setPosition(Vector3 newPosition)
{
	m_position = newPosition;
}

Vector3 Camera::getPosition()
{
	return m_position;
}

Vector3 Camera::getForward()
{
	return m_forward;
}

void Camera::setRotation(Vector3 newRotation)
{
	m_orientation = newRotation;
}

Vector3 Camera::getRotation()
{
	return m_orientation;
}

float Camera::getMoveSpeed()
{
	return m_movespeed;
}

float Camera::getRotationSpeed()
{
	return m_camRotRate;
}

Vector3 Camera::getUp()
{
	return m_up;
}

Vector3 Camera::getRight()
{
	return m_right;
}