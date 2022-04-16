#include "pch.h"
#include "CameraMovement.h"


CameraMovement::~CameraMovement()
{
}

DirectX::SimpleMath::Vector3 CameraMovement::UpdateCameraMovement(Camera _cam, RayTriangleIntersection _rayTriIntersect, Box* box) {

	DirectX::SimpleMath::Vector3 camPos = _cam.getPosition()*10;
	DirectX::SimpleMath::Vector3 rayOrigination = camPos + DirectX::SimpleMath::Vector3(0, 100, 0);
	DirectX::SimpleMath::Vector3 rayDestination = camPos - DirectX::SimpleMath::Vector3(0, 100, 0);
	if (box == NULL) {
		return _cam.getPosition();
	}
	Triangle triangle = _rayTriIntersect.CheckIntersectsTriangles(*box, rayOrigination, rayDestination);
    DirectX::SimpleMath::Vector3 mid = (triangle.trianglePositions[0] + triangle.trianglePositions[1] + triangle.trianglePositions[2]) / 3;
    DirectX::SimpleMath::Vector3 scaleAccounted = DirectX::SimpleMath::Vector3(mid.x/10, mid.y/10, mid.z/10);
;
	if (triangle.trianglePositions[0] != DirectX::SimpleMath::Vector3(INT64_MIN)&&triangle.trianglePositions[0]!= DirectX::SimpleMath::Vector3(0,0,0)) {
		
		
		return BarycentricCoords(triangle, camPos);	 //return scaleAccounted + DirectX::SimpleMath::Vector3(0, 0.01, 0);

	}
	else return _cam.getPosition();
	
}

DirectX::SimpleMath::Vector3 CameraMovement::BarycentricCoords(Triangle _triangle, DirectX::SimpleMath::Vector3 _currentPos) {
	float weight1 = FindWeight1(_currentPos, _triangle.trianglePositions[0], _triangle.trianglePositions[1], _triangle.trianglePositions[2]);
	float weight2 = FindWeight2(_currentPos, _triangle.trianglePositions[0], _triangle.trianglePositions[1], _triangle.trianglePositions[2]);
	float weight3 = FindWeight3(weight1, weight2);
	float yVal = (weight1 * _triangle.trianglePositions[0].y) + (weight2 * _triangle.trianglePositions[1].y) + (weight3 * _triangle.trianglePositions[2].y);
	return DirectX::SimpleMath::Vector3(0, yVal/10, 0);
}
float CameraMovement::FindWeight1(DirectX::SimpleMath::Vector3 _curPos, DirectX::SimpleMath::Vector3 p1
	, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3) {
	float topLine = ((p2.z - p3.z) * (_curPos.x - p3.x)) + ((p3.x - p2.x) * (_curPos.z - p3.z));
	float bottomLine = ((p2.z - p3.z) * (p1.x- p3.x)) + ((p3.x - p2.x) * (p1.z - p3.z));
	return (topLine / bottomLine);
}
float CameraMovement::FindWeight2(DirectX::SimpleMath::Vector3 _curPos, DirectX::SimpleMath::Vector3 p1
	, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3) {
	float topLine = ((p3.z - p1.z) * (_curPos.x - p3.x)) + ((p1.x - p3.x) * (_curPos.z - p3.z));
	float bottomLine = ((p2.z - p3.z) * (p1.x - p3.x)) + ((p3.x - p2.x) * (p1.z - p3.z));
	return (topLine / bottomLine);
}
float CameraMovement::FindWeight3(float weight1, float weight2) {
	return 1 - weight2 - weight1;
}

