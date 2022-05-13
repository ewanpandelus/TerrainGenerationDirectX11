#include "pch.h"
#include "PlacedObjects.h"


PlacedObjects::~PlacedObjects()
{
}


void PlacedObjects::AddToObjectPositions(SimpleMath::Vector3 objectPosition) {
	PlacedObjectType placedObj;
	placedObj.position = objectPosition;
	placedObj.colour[0] = m_selectedColour[0];
	placedObj.colour[1] = m_selectedColour[1];
	placedObj.colour[2] = m_selectedColour[2];

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
	for (int i = 0; i < 7; i++)
	{
		CoinObjectType coin;
		coin.position = SimpleMath::Vector3(5 + rand() % 100, terrainY + 1 + rand() % 4, 5 + rand() % 100);
		m_coins.push_back(coin);
	}
}
std::vector<PlacedObjects::CoinObjectType> PlacedObjects::GetCoins() {
	return m_coins;
}
void PlacedObjects::ClearCoinPositions()
{
	m_coins.clear();
}
void PlacedObjects::RemoveCoin(int index)
{
	m_coins.erase(m_coins.begin() + index);
}
void PlacedObjects::AssignCollected(int index)
{
	m_coins[index].collected = true;
}
void PlacedObjects::DecreaseCoinScale()
{
	int index = 0;
	int deleteIndex = -1;
	for each (CoinObjectType var in m_coins)
	{
		if (var.collected == true)
		{
			m_coins[index].scale -= 0.05f;
		}
		if (var.scale <= 0) deleteIndex = index;
		index++;
	}
	if (deleteIndex >= 0) {
		RemoveCoin(deleteIndex);
	}
}
float* PlacedObjects::SetSelectedColour()
{
	return &m_selectedColour[0];
}