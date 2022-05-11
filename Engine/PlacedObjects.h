#pragma once
using namespace DirectX;

class PlacedObjects
{
public:

	~PlacedObjects();
	void AddToObjectPositions(SimpleMath::Vector3 objectPositions);
	void ClearObjectPositions();
	std::vector<SimpleMath::Vector3> GetObjectPositions();
private:
	std::vector<SimpleMath::Vector3> m_objectPositions;
};

