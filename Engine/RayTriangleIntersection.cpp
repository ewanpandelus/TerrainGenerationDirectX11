#include "pch.h"
#include "RayTriangleIntersection.h"


RayTriangleIntersection::~RayTriangleIntersection()
{
}


Triangle RayTriangleIntersection::CheckIntersectsTriangles(Box box,
	DirectX::SimpleMath::Vector3 rayOrigination, DirectX::SimpleMath::Vector3 rayDestination)
{
	for each (Triangle var in box.triangles)
	{
		if (Intersects(rayOrigination, rayDestination-rayOrigination, var.trianglePositions[0], var.trianglePositions[1], var.trianglePositions[2], 1)) {
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
bool RayTriangleIntersection::Intersects(DirectX::SimpleMath::Vector3 orig, DirectX::SimpleMath::Vector3  dir,
	DirectX::SimpleMath::Vector3  v0, DirectX::SimpleMath::Vector3 v1, DirectX::SimpleMath::Vector3 v2,
	float t) {
	float kEpsilon = 0.00001f;

	DirectX::SimpleMath::Vector3 v0v1 = v1 - v0;
	DirectX::SimpleMath::Vector3 v0v2 = v2 - v0;
	// no need to normalize
	DirectX::SimpleMath::Vector3 N = v0v1.Cross(v0v2); // N 
	float area2 = N.Length();

	// Step 1: finding P

	// check if ray and plane are parallel ?
	float NdotRayDirection = N.Dot(dir);
	if (fabs(NdotRayDirection) < kEpsilon) // almost 0 
		return false; // they are parallel so they don't intersect ! 

	// compute d parameter using equation 2
	float d = -N.Dot(v0);

	// compute t (equation 3)
	t = -(N.Dot(orig) + d) / NdotRayDirection;

	// check if the triangle is in behind the ray
	if (t < 0) return false; // the triangle is behind 

	// compute the intersection point using equation 1
	DirectX::SimpleMath::Vector3 P = orig + t * dir;

	// Step 2: inside-outside test
	DirectX::SimpleMath::Vector3 C; // vector perpendicular to triangle's plane 

	// edge 0
	DirectX::SimpleMath::Vector3 edge0 = v1 - v0;
	DirectX::SimpleMath::Vector3 vp0 = P - v0;
	C = edge0.Cross(vp0);
	if (N.Dot(C) < 0) return false; // P is on the right side 

	// edge 1
	DirectX::SimpleMath::Vector3 edge1 = v2 - v1;
	DirectX::SimpleMath::Vector3 vp1 = P - v1;
	C = edge1.Cross(vp1);
	if (N.Dot(C) < 0)  return false; // P is on the right side 

	// edge 2
	DirectX::SimpleMath::Vector3 edge2 = v0 - v2;
	DirectX::SimpleMath::Vector3 vp2 = P - v2;
	C = edge2.Cross(vp2);
	if (N.Dot(C) < 0) return false; // P is on the right side; 

	return true; // this ray hits the triangle 
}

