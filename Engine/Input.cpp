#include "pch.h"
#include "Input.h"
#include "Mouse.h"
using namespace DirectX::SimpleMath;
using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
Vector2 oldPosition = Vector2(0, 0);

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

	m_GameInput.forward = false;
	m_GameInput.back = false;
	m_GameInput.right = false;
	m_GameInput.left = false;
	m_GameInput.rotRight = false;
	m_GameInput.rotLeft = false;
	m_GameInput.rotUp = false;
	m_GameInput.rotDown = false;
	m_GameInput.generate = false;
	m_GameInput.smooth = false;
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
	if (kb.Left)	m_GameInput.rotLeft = true;
	else		m_GameInput.rotLeft = false;

	//D key
	if (kb.Right)	m_GameInput.rotRight = true;
	else		m_GameInput.rotRight = false;

	//W key
	if (kb.Up)	m_GameInput.rotUp = true;
	else		m_GameInput.rotUp = false;

	//S key
	if (kb.Down)	m_GameInput.rotDown = true;
	else			m_GameInput.rotDown = false;

	//Down arrow key
	if (kb.S)	 m_GameInput.back = true;
	else		 m_GameInput.back = false;

	//Up arrow key
	if (kb.W)	 m_GameInput.forward = true;
	else		 m_GameInput.forward = false;

	//Left arrow key
	if (kb.A)	 m_GameInput.left = true;
	else		 m_GameInput.left = false;

	//Right arrow key
	if (kb.D)	 m_GameInput.right = true;
	else		 m_GameInput.right = false;

	//Space key
	if (kb.Space) m_GameInput.up = true;
	else		  m_GameInput.up = false;

	//X key
	if (kb.X)		m_GameInput.down = true;
	else			m_GameInput.down = false;

	//R key
	if (kb.R)		m_GameInput.reset = true;
	else			m_GameInput.reset = false;

	//F key
	if (kb.F) m_GameInput.smooth = true;
	else m_GameInput.smooth = false;

	//G key
	if (kb.G) m_GameInput.generate = true;
	else m_GameInput.generate = false;

	Vector2 newPosition = { (float)mouse.x, (float)mouse.y };

	if (mouse.leftButton && m_MouseTracker.leftButton == ButtonState::HELD) {

		const float deltaX = newPosition.x - oldPosition.x;
		const float deltaY = newPosition.y - oldPosition.y;

		if (deltaX > 0) m_GameInput.rotRight = true;
		else m_GameInput.rotRight = false;

		if (deltaX < 0) m_GameInput.rotLeft = true;
		else m_GameInput.rotLeft = false;

		if (deltaY < 0) m_GameInput.rotUp = true;
		else m_GameInput.rotUp = false;

		if (deltaY > 0) m_GameInput.rotDown = true;
		else m_GameInput.rotDown = false;

	}

	oldPosition = newPosition;

}

bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}
