#pragma once
#include "MeshShapes.h"



using namespace DirectX;

class RayTriangleIntersection
{
public:

	~RayTriangleIntersection();

	bool Intersects(DirectX::SimpleMath::Vector3 rayOrigination,
		DirectX::SimpleMath::Vector3 rayDestination, DirectX::SimpleMath::
		Vector3 triVertex1, DirectX::SimpleMath::Vector3 triVertex2,
		DirectX::SimpleMath::Vector3 triVertex3, Triangle triangle);
	Triangle CheckIntersectsTriangles(Box box,
		DirectX::SimpleMath::Vector3 rayOrigination,
		DirectX::SimpleMath::Vector3 rayDestination);
	SimpleMath::Vector3 TriMidPoint(Triangle tri);
	bool Intersects2(DirectX::SimpleMath::Vector3 orig, DirectX::SimpleMath::Vector3  dir,
		DirectX::SimpleMath::Vector3  v0, DirectX::SimpleMath::Vector3 v1, DirectX::SimpleMath::Vector3 v2,
		float t);
private:

};

