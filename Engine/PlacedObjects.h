#pragma once
using namespace DirectX;

class PlacedObjects
{
public:

	~PlacedObjects();
	struct PlacedObjectType
	{
		SimpleMath::Vector3 position;
		float scale = 1;

	};
	void AddToObjectPositions(SimpleMath::Vector3 objectPositions);
	void ClearObjectPositions();
	std::vector<PlacedObjectType> GetObjectPositions();

	void AddToCoinPositions(float terrainY);
	void ClearCoinPositions();
	std::vector<SimpleMath::Vector3> GetCoinPositions();
	
private:

	std::vector<PlacedObjectType> m_objectPositions;
	std::vector<SimpleMath::Vector3> m_coinPositions;
	

};

