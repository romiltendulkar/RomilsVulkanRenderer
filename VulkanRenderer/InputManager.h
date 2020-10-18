#pragma once

#define IM_KEYBOARD_SIZE 130
#include <vector>
#include <GLFW/glfw3.h>

class InputManager
{
public:
	

	InputManager();
	~InputManager();
	void Update();

	//////////////////////////////////////////////////////////////
	// Key Press Functions
	//////////////////////////////////////////////////////////////
	bool IsPressed(int key);
	bool IsTriggered(int key);
	bool IsReleased(int key);
	void SetWindow(GLFWwindow* mpWindow);

	//////////////////////////////////////////////////////////////
	// Variables
	//////////////////////////////////////////////////////////////
public:
	GLFWwindow* mpWindow;
private:
	std::vector<bool> mCurrentKeys;
	std::vector<bool> mPrevKeys;
};