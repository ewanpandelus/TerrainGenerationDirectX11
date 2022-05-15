#pragma once
class PostProcessEffects
{
public:
	PostProcessEffects();
	~PostProcessEffects();
	float* SetBloomBrightness();
	float  GetBloomBrightness();
	
	float* SetBloomBlurRadius();
	float GetBloomBlurStength();

	bool* SetPostProcessImGUI();
	bool  GetPostProcess();
private:
	float m_bloomBrightness =1.4;
	float m_bloomBlurStrength = 0.5;
	bool m_postProcess = false;
};

