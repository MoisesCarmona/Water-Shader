#pragma once

#include "XTime.h"
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

void ortho_normalize(XMMATRIX& Outmatrix)
{
	XMVECTOR Up = { 0,1,0,1 };
	XMVECTOR Z = Outmatrix.r[2];
	Z = XMVector4Normalize(Z);
	XMVECTOR X = XMVector3Cross(Up, Z);
	X = XMVector4Normalize(X);
	XMVECTOR Y = XMVector3Cross(Z, X);
	Y = XMVector4Normalize(Y);
	Outmatrix = { X,Y,Z,Outmatrix.r[3] };

}
XTime timeX;
POINT p = POINT{ 0,0 };
POINT m;
void camera_move(XMMATRIX& Camera)
{
	Camera = (XMMatrixInverse(nullptr, Camera));
	static float speed = 10.0f;
	timeX.Signal();
	if (GetAsyncKeyState(VK_SHIFT))
	{
		speed = 100.0f;

	}
	else {
		speed = 10.0f;
	}
	XMMATRIX vMatrix = Camera;
	{

		m = p;
		GetCursorPos(&p);


		float dX = p.x - m.x;
		float dY = p.y - m.y;
		XMMATRIX rotateY, rotateX = XMMatrixIdentity();

		rotateY = XMMatrixRotationY(dX * 2 * timeX.Delta());
		vMatrix = XMMatrixMultiply(rotateY, vMatrix);

		Camera = vMatrix;

		rotateX = XMMatrixRotationX(dY * 2 * timeX.Delta());
		vMatrix = XMMatrixMultiply(rotateX, vMatrix);

		Camera = vMatrix;


	}
	if (GetAsyncKeyState(0x57))//W
	{

		XMMATRIX translate = XMMatrixTranslation(0, 0, speed);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(0x53))//S
	{
		XMMATRIX translate = XMMatrixTranslation(0, 0, -speed);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(0x41))//A
	{
		XMMATRIX translate = XMMatrixTranslation(-speed, 0, 0);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(0x44))//D
	{
		XMMATRIX translate = XMMatrixTranslation(speed, 0, 0);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		XMMATRIX translate = XMMatrixTranslation(0, speed, 0);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(VK_CONTROL))
	{
		XMMATRIX translate = XMMatrixTranslation(0, -speed, 0);
		vMatrix = XMMatrixMultiply(translate, vMatrix);
		Camera = vMatrix;

	}
	if (GetAsyncKeyState(0x52))
	{



	}
	ortho_normalize(Camera);
	Camera = (XMMatrixInverse(nullptr, Camera));


}

void GetRotation(float& Yaw, float& Pitch, float& Roll, XMMATRIX& Camera) //y,x,z
{
	if (Camera.r[0].m128_f32[0] == 1.0f)
	{
		Yaw = atan2f(Camera.r[0].m128_f32[2], Camera.r[2].m128_f32[3]);
		Pitch = 0;
		Roll = 0;

	}
	else if (Camera.r[0].m128_f32[0] == -1.0f)
	{
		Yaw = atan2f(Camera.r[0].m128_f32[2], Camera.r[2].m128_f32[3]);
		Pitch = 0;
		Roll = 0;
	}
	else
	{

		Yaw = atan2(-Camera.r[2].m128_f32[0], Camera.r[0].m128_f32[0]);
		Pitch = asin(Camera.r[1].m128_f32[0]);
		Roll = atan2(-Camera.r[1].m128_f32[2], Camera.r[1].m128_f32[1]);
	}
}
XMMATRIX RenderReflection(XMMATRIX& Camera, float height)
{
	XMMATRIX m_reflectionViewMatrix = Camera;
	m_reflectionViewMatrix = (XMMatrixInverse(nullptr, m_reflectionViewMatrix));

	XMMATRIX reflectionMatrix_Y =
	{ 
		{1,0,0,0 },
		{0,-1,0,0 },
		{0,0,1,0 },
		{0,0,0,1 },
	};
	m_reflectionViewMatrix =	XMMatrixMultiply(m_reflectionViewMatrix, reflectionMatrix_Y);

	m_reflectionViewMatrix = (XMMatrixInverse(nullptr, m_reflectionViewMatrix));


	return m_reflectionViewMatrix;



}
