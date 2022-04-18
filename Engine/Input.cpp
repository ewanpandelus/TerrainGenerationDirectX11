#include "pch.h"
#include "Input.h"


Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_quitApp = false;

	m_GameInput.forward		= false;
	m_GameInput.back		= false;
	m_GameInput.right		= false;
	m_GameInput.left		= false;
	m_GameInput.rotRight	= false;
	m_GameInput.rotLeft		= true;
	m_GameInput.leftMouse = false;
	m_GameInput.p = false;


	prevX = 0;
	xChange = 0;
	prevY = 0;
	yChange = 0;
}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 
	
	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	//A key
	if (kb.A)	m_GameInput.left = true;
	else		m_GameInput.left = false;
	
	//D key
	if (kb.D)	m_GameInput.right = true;
	else		m_GameInput.right = false;

	//W key
	if (kb.W)	m_GameInput.forward	 = true;
	else		m_GameInput.forward = false;

	//S key
	if (kb.S)	m_GameInput.back = true;
	else		m_GameInput.back = false;

	//space
	if (kb.Space) m_GameInput.generate = true;
	else		m_GameInput.generate = false;

	if (mouse.leftButton) m_GameInput.leftMouse = true;
	else m_GameInput.leftMouse = false;
	
	if (kb.P) m_GameInput.p = true;
	else m_GameInput.p = false;

	float newX = mouse.x;
	if (prevX != 0) {
		if (newX != prevX) {
			xChange = newX - prevX;
		}
	}
	float newY = mouse.y;
	if (prevY != 0) {
		if (newY != prevY) {
			yChange = newY - prevY;
		}
	}
	prevX = mouse.x;
	prevY = mouse.y;
}
int::Input::GetMouseX() {
	return m_mouse->GetState().x;
}
int::Input::GetMouseY() {
	return m_mouse->GetState().y;
}
bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}
float Input::getXChange() {
	return xChange;
}
float Input::getYChange() {
	return yChange;
}
