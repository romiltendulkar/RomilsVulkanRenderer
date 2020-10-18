#include "InputManager.h"

InputManager::InputManager()
{

	//create a std vector of size IM_KEYBOARD_SIZE
	mPrevKeys = std::vector<bool>(IM_KEYBOARD_SIZE);
	mCurrentKeys = std::vector<bool>(IM_KEYBOARD_SIZE);

	//cycle through and set everything to false
	for (int counter = 0; counter < IM_KEYBOARD_SIZE; ++counter)
	{
		mPrevKeys[counter] = false;
		mCurrentKeys[counter] = false;
	}
}

InputManager::~InputManager()
{

	mPrevKeys.clear();
	mCurrentKeys.clear();
}


void InputManager::Update()
{
	//updates the data structure containing which keys are pressed.
	for (int counter = 0; counter < IM_KEYBOARD_SIZE; ++counter)
	{
		mPrevKeys[counter] = mCurrentKeys[counter];

		//the glfwGetKey callback is required to update current state of that key.
		//these are not updated otherwise
		if (glfwGetKey(mpWindow, counter + 32) == GLFW_PRESS)
		{
			mCurrentKeys[counter] = true;
		}
		else
		{
			mCurrentKeys[counter] = false;
		}

	}
}

void InputManager::SetWindow(GLFWwindow* Window)
{
	mpWindow = Window;
}

bool InputManager::IsPressed(int key)
{
	//the keys we are concerned with start at 32,
	//but our vector starts with 0
	int location = key - 32;
	if (mCurrentKeys[location] == true)
	{
		return true;
	}
	return false;
}

bool InputManager::IsTriggered(int key)
{
	//the keys we are concerned with start at 32,
	//but our vector starts with 0
	int location = key - 32;
	if (mPrevKeys[location] == false && mCurrentKeys[location] == true)
	{
		return true;
	}
	return false;
}

bool InputManager::IsReleased(int key)
{
	//the keys we are concerned with start at 32,
	//but our vector starts with 0
	int location = key - 32;
	if (mPrevKeys[location] == true && mCurrentKeys[location] == false)
	{
		return true;
	}
	return false;
}