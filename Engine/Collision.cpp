#include "pch.h"
#include "Collision.h"



Collision::~Collision()
{
}

bool Collision::Collides(DirectX::SimpleMath::Vector3 playerPos, DirectX::SimpleMath::Vector3 objectPos,
    float objectWidth, float objectHeight, float playerWidth,float  playerHeight) 
{
    return (playerPos.x < objectPos.x + objectWidth &&
        playerPos.x + playerWidth > objectPos.x &&
        playerPos.z < objectPos.z + objectHeight &&
        playerPos.z + playerHeight> objectPos.z);
  
}





