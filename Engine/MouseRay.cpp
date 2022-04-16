#include "pch.h"
#include "MouseRay.h"



MouseRay::~MouseRay()
{
}

DirectX::SimpleMath::Vector3 MouseRay::GetDirectionVectorOfMouse(int mouseX, int mouseY, int screenWidth, int screenHeight,
	DirectX::SimpleMath::Matrix worldMatrix, DirectX::SimpleMath::Matrix viewMatrix, DirectX::SimpleMath::Matrix projectionMatrix,
	DirectX::SimpleMath::Vector3 origin) {
	float pointX, pointY;
	//D3DXMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
	pointX = ((2.0f * (float)mouseX) / (float)screenWidth) - 1.0f;
	pointY = (((2.0f * (float)mouseY) / (float)screenHeight) - 1.0f) * -1.0f;
		
	// Adjust the points using the projection matrix to account for the aspect ratio of the viewport.

	pointX = pointX / projectionMatrix._11;
	pointY = pointY / projectionMatrix._22;
	viewMatrix.Invert();
	// Get the inverse of the view matrix.

	DirectX::SimpleMath::Matrix inverseViewMatrix = viewMatrix;
	
	DirectX::SimpleMath::Vector3 direction, rayOrigin, rayDirection;
	// Calculate the direction of the picking ray in view space.
	direction.x = (pointX * inverseViewMatrix._11) + (pointY * inverseViewMatrix._21) + inverseViewMatrix._31;
	direction.y = (pointX * inverseViewMatrix._12) + (pointY * inverseViewMatrix._22) + inverseViewMatrix._32;
	direction.z = (pointX * inverseViewMatrix._13) + (pointY * inverseViewMatrix._23) + inverseViewMatrix._33;

	// Get the world matrix and translate to the location of the sphere.

	rayOrigin = DirectX::SimpleMath::Vector3::Transform(origin, inverseViewMatrix);
	rayDirection = DirectX::SimpleMath::Vector3::TransformNormal(direction, worldMatrix.Invert());

	// Normalize the ray direction.
	rayDirection.Normalize();


	return rayDirection;
}




