#include "pch.h"
#include "PlacedObjects.h"


PlacedObjects::~PlacedObjects()
{
}


void PlacedObjects::AddToObjectPositions(SimpleMath::Vector3 objectPosition) {
	m_objectPositions.push_back(objectPosition);
}
std::vector<SimpleMath::Vector3> PlacedObjects::GetObjectPositions() {
	return m_objectPositions;
}
void PlacedObjects::ClearObjectPositions()
{
	m_objectPositions.clear();
}
