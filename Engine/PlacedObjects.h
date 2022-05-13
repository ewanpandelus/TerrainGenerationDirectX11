#pragma once
using namespace DirectX;

class PlacedObjects
{
public:

	~PlacedObjects();
	struct CoinObjectType
	{
		SimpleMath::Vector3 position;
		float scale = 0.4;
		bool collected = false;
	};
	struct PlacedObjectType
	{
		SimpleMath::Vector3 position;
		float colour[3] = {0.2,0.6,0.2};
		float scale = 1;
	};

	void AddToObjectPositions(SimpleMath::Vector3 objectPositions);
	void ClearObjectPositions();
	std::vector<PlacedObjectType> GetObjectPositions();

	void AddToCoinPositions(float terrainY);
	void ClearCoinPositions();
	std::vector<CoinObjectType> GetCoins();
	void RemoveCoin(int index);
	void DecreaseCoinScale();
	void AssignCollected(int index);
	float* SetSelectedColour();

private:

	std::vector<PlacedObjectType> m_objectPositions;
	std::vector<CoinObjectType> m_coins;
	float m_selectedColour[3] = { 1,1,1 };


};

