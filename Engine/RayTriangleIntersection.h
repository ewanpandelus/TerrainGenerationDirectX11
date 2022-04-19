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
private:

};

