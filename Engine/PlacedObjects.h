#pragma once
using namespace DirectX;

class PlacedObjects
{
public:

	~PlacedObjects();
	void Initialise();
	struct CoinObjectType
	{
		SimpleMath::Vector3 position;
		float scale = 0.4;
		bool collected = false;
	};
	struct PlacedObjectType
	{
		SimpleMath::Vector3 position;
		short type = 1;
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
	float* GetTreeColour();
	float* GetTree1Colour();
	float* GetTree2Colour();
	int* SetCurrentTreeSelected();
	bool* GetPlaceTrees();


private:

	std::vector<PlacedObjectType> m_objectPositions;
	std::vector<CoinObjectType> m_coins;
	std::vector<float*> m_treeColours;
	float m_treeColour[3] = { 1,1,1 };
	float m_tree1Colour[3] = { 1,1,1 };
	float m_tree2Colour[3] = { 1,1,1 };
	bool m_placeTrees = false;
	int m_currentTreeSelected = 0;

};

