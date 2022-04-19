#include "pch.h"
#include "RayTriangleIntersection.h"


RayTriangleIntersection::~RayTriangleIntersection()
{
}

bool RayTriangleIntersection::Intersects(DirectX::SimpleMath::Vector3 rayOrigination,
	DirectX::SimpleMath::Vector3 rayDestination, DirectX::SimpleMath::
	Vector3 triVertex0, DirectX::SimpleMath::Vector3 triVertex1,
	DirectX::SimpleMath::Vector3 triVertex2, Triangle triangle)
{
	float epslion = 0.00001f;
	triVertex1 /= 10;
	triVertex2 /= 10;
	triVertex0 /= 10;
	DirectX::SimpleMath::Vector3 e1 = triVertex1 - triVertex0;
	DirectX::SimpleMath::Vector3 e2 = triVertex2 - triVertex0;
	DirectX::SimpleMath::Vector3 q;
	rayDestination.Cross(e2, q);
	float a = e1.Dot(q);
	if (a > -epslion && a < epslion) {
		return false;
	}

	float f = 1 / a;
	DirectX::SimpleMath::Vector3 s = rayOrigination - triVertex0;

	float u = f * (s.Dot(q));
	if (u < 0) {
		return false;

	}

	DirectX::SimpleMath::Vector3 r;
	s.Cross(e1, r);

	float v = f * (rayDestination.Dot(r));

	if (u < 0 || u + v > 1) {
		return false;
	}

	float t = f * (e2.Dot(r));
	return true;
}
Triangle RayTriangleIntersection::CheckIntersectsTriangles(Box box,
	DirectX::SimpleMath::Vector3 rayOrigination, DirectX::SimpleMath::Vector3 rayDestination)
{
	for each (Triangle var in box.triangles)
	{
		if (Intersects(rayOrigination, rayDestination, var.trianglePositions[0], var.trianglePositions[1], var.trianglePositions[2], var)) {
			return var;
		}
	}
	Triangle triangle;
	DirectX::SimpleMath::Vector3 undefined = DirectX::SimpleMath::Vector3(INT64_MIN);
	return triangle;
}
SimpleMath::Vector3 RayTriangleIntersection::TriMidPoint(Triangle tri) {
	DirectX::SimpleMath::Vector3 mid = (tri.trianglePositions[0] + tri.trianglePositions[1] + tri.trianglePositions[2]);
	DirectX::SimpleMath::Vector3 scaleAccounted = DirectX::SimpleMath::Vector3(mid.x, mid.y, mid.z);
	return scaleAccounted;
}

