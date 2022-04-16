
#pragma once
#include "pch.h"
using namespace DirectX;
class MeshShapes
{

public:
	MeshShapes();
	~MeshShapes();

};
struct Triangle {
	DirectX::SimpleMath::Vector3 trianglePositions[3];

};
struct Box {
	Triangle triangles[2];
	float xMin;
	float xMax;
	float zMin;
	float zMax;
};

