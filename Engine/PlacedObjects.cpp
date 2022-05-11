#include "pch.h"
#include "PlacedObjects.h"


PlacedObjects::~PlacedObjects()
{
}


void PlacedObjects::AddToObjectPositions(SimpleMath::Vector3 objectPosition) {
	PlacedObjectType placedObj;
	placedObj.position = objectPosition;
//	placedObj.scale = 1 + (rand() % 2) / 10.0;

	m_objectPositions.push_back(placedObj);
	
}
std::vector<PlacedObjects::PlacedObjectType> PlacedObjects::GetObjectPositions() {
	return m_objectPositions;
}

void PlacedObjects::ClearObjectPositions()
{
	m_objectPositions.clear();
}


void PlacedObjects::AddToCoinPositions(float terrainY) {
	for (int i = 0; i < 4; i++) 
	{
		m_coinPositions.push_back(SimpleMath::Vector3(5 + rand() % 100, terrainY + 1 + rand() % 4, 5 + rand() % 100));
	}
}
std::vector<SimpleMath::Vector3> PlacedObjects::GetCoinPositions() {
	return m_coinPositions;
}
void PlacedObjects::ClearCoinPositions()
{
	m_coinPositions.clear();
}