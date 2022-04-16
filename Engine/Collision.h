#pragma once
#include "pch.h"



using namespace DirectX;

class Collision
{
public:

	~Collision();

	bool Collides(DirectX::SimpleMath::Vector3 playerPos, DirectX::SimpleMath::Vector3 objectPos,
		float objectWidth, float objectHeight, float playerWidth, float  playerHeight);
		
		
private:

};

