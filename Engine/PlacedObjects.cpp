#include "pch.h"
#include "PlacedObjects.h"


PlacedObjects::~PlacedObjects()
{

}
void PlacedObjects::Initialise()
{
	m_treeColours.push_back(&m_treeColour[0]);
	m_treeColours.push_back(&m_tree1Colour[0]);
	m_treeColours.push_back(&m_tree1Colour[2]);
	AddToCoinPositions(0);

}

void PlacedObjects::AddToObjectPositions(SimpleMath::Vector3 objectPosition) {
	PlacedObjectType placedObj;
	placedObj.position = objectPosition;
	placedObj.type = m_currentTreeSelected;


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
bool PlacedObjects::DecreaseCoinScale()
{
	int index = 0;
	int deleteIndex = -1;
	for each (CoinObjectType var in m_coins)
	{
		if (var.collected == true) { m_coins[index].scale -= 0.05f; }
		if (var.scale <= 0) { deleteIndex = index; }

		index++;
	}
	if (deleteIndex >= 0) {
		RemoveCoin(deleteIndex);
	}
	return (m_coins.size() == 0);
}

bool* PlacedObjects::GetPlaceTrees() {
	return &m_placeTrees;
}
float* PlacedObjects::GetTreeColour()
{
	return &m_treeColour[0];
}
float* PlacedObjects::GetTree1Colour()
{
	return &m_tree1Colour[0];
}
float* PlacedObjects::GetTree2Colour()
{
	return &m_tree2Colour[0];
}
int* PlacedObjects::SetCurrentTreeSelected()
{
	return &m_currentTreeSelected;
}
