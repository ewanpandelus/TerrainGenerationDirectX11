#pragma once
#include "RayTriangleIntersection.h"
#include "Camera.h"


using namespace DirectX;

class CameraMovement
{
public:

	~CameraMovement();
	DirectX::SimpleMath::Vector3 UpdateCameraMovement(Camera _cam, RayTriangleIntersection _rayTriIntersect, Box* _box);
	
private:
	DirectX::SimpleMath::Vector3 BarycentricCoords(Triangle _triangle, DirectX::SimpleMath::Vector3 _curPos);
	float FindWeight1(DirectX::SimpleMath::Vector3 _curPos, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3);
	float FindWeight2(DirectX::SimpleMath::Vector3 _curPos, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3);
	float FindWeight3(float weight1, float weight2);
};

