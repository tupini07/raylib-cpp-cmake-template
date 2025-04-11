#pragma once
#include <raylib.h>
#include <string>

#include "../BaseScene.hpp"

class TitleScene : public BaseScene
{
private:
	// Assets
	Texture2D texture;
	
	// Virtual mouse handling
	Vector2 virtualMousePosition;
	
	// RayGUI component states
	// Button component: Uses return value, no state variable needed
	
	// Checkbox component
	bool checkboxState;
	
	// Dropdown component
	int dropdownIndex;
	bool dropdownEditMode;
	
	// Text input component
	char textBoxText[64];
	bool textBoxFocused;
	
	// Color picker component
	Color colorPickerValue;
	
	// Message box component
	bool showMessageBox;
	bool messageBoxOkClicked;

public:
	TitleScene();
	~TitleScene();

	Scenes tick(float dt) override;
};
