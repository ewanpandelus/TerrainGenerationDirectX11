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
	float m_bloomBrightness =0.1;
	float m_bloomBlurStrength = 0.1;
	bool m_postProcess = false;
};

