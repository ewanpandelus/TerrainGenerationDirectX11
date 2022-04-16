#pragma once

class MouseRay
{
private:









public:

	~MouseRay();


	DirectX::SimpleMath::Vector3 GetDirectionVectorOfMouse(int mouseX, int mouseY, int screenWidth, int screenHeight, 
		DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix projection, DirectX::SimpleMath::Vector3 origin);

	

private:





};

