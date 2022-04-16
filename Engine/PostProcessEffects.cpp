#include "pch.h"
#include "PostProcessEffects.h"

PostProcessEffects::PostProcessEffects()
{
	
}


PostProcessEffects::~PostProcessEffects()
{
}


float PostProcessEffects::GetBloomBrightness()
{
	return m_bloomBrightness;

}
float* PostProcessEffects::SetBloomBrightness()
{
	return &m_bloomBrightness;

}
float PostProcessEffects::GetBloomBlurStength()
{
	return m_bloomBlurStrength;

}
float* PostProcessEffects::SetBloomBlurRadius()
{
	return &m_bloomBlurStrength;

}
bool* PostProcessEffects::SetPostProcessImGUI()
{
	return &m_postProcess;
}
bool PostProcessEffects::GetPostProcess()
{
	return m_postProcess;
}
