#include <string>

#include <raylib.h>
#include <raygui.h>

#include <Constants.hpp>
#include <utils/DebugUtils.hpp>

#include "TitleScene.hpp"
#include "../Scenes.hpp"

using namespace std;

TitleScene::TitleScene()
{
	// Load assets
	texture = LoadTexture(AppConstants::GetAssetPath("test.png").c_str());

	// Initialize GUI component states
	checkboxState = false;
	dropdownIndex = 0;
	dropdownEditMode = false;
	strcpy(textBoxText, "Edit me!");
	textBoxFocused = false;
	colorPickerValue = RED;
	showMessageBox = false;
	messageBoxOkClicked = false;

	// Initialize virtual mouse (scaled for GUI components)
	SetMouseScale(1.0f / (float)ScreenScale, 1.0f / (float)ScreenScale);
	HideCursor(); // Hide OS cursor since we'll draw our own
}

TitleScene::~TitleScene()
{
	// Restore default mouse settings when exiting the scene
	SetMouseScale(1.0f, 1.0f);
	SetMouseOffset(0, 0);
	ShowCursor();

	// Unload assets
	UnloadTexture(texture);
}

Scenes TitleScene::tick(float dt)
{
	// Clear background and prepare for drawing
	ClearBackground(RAYWHITE);
	virtualMousePosition = GetMousePosition();

	auto draw_with_backdrop = [](const string &text, int x, int y, int fontSize, Color color, Color backdropColor)
	{
		DrawText(text.c_str(), x + 1, y + 1, fontSize, backdropColor);
		DrawText(text.c_str(), x, y, fontSize, color);
	};

	draw_with_backdrop("This is the Title Scene", 10, 10, 25, GOLD, BLACK);

	// Draw centered texture
	const int texture_x = (GameConstants::WorldWidth / 2) - (texture.width / 2);
	const int texture_y = (GameConstants::WorldHeight / 2) - (texture.height / 2) + 30;
	DrawTextureEx(texture, Vector2{(float)texture_x, (float)texture_y}, 0, 1, WHITE);

	// Set smaller text size for GUI components
	GuiSetStyle(DEFAULT, TEXT_SIZE, 10);

	// Define panel positions
	const int leftPanel = 10;
	const int rightPanel = GameConstants::WorldWidth - 160;

	// Save initial style properties
	const int defaultTextSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const int defaultColorPickerSize = GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE);

	//=================================================================
	// LEFT PANEL - BASIC GUI COMPONENTS
	//=================================================================
	draw_with_backdrop("RayGUI Examples", leftPanel, 70, 15, BLACK, WHITE);

	if( GuiButton(Rectangle{leftPanel, 95, 120, 30}, "Start Game")){
		// Transition to the game scene
		return Scenes::GAME;
	}

	GuiCheckBox(Rectangle{leftPanel, 135, 20, 20}, "Enable Feature", &checkboxState);

	Rectangle textBoxRect = {leftPanel, 165, 120, 25};
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		// Update focus state when mouse is clicked
		textBoxFocused = CheckCollisionPointRec(GetMousePosition(), textBoxRect);
	}
	GuiTextBox(textBoxRect, textBoxText, 64, textBoxFocused);

	Rectangle dropdownRect = {leftPanel, 200, 120, 25};
	if (GuiDropdownBox(dropdownRect, "Option 1;Option 2;Option 3", &dropdownIndex, dropdownEditMode))
	{
		dropdownEditMode = !dropdownEditMode;
	}

	//=================================================================
	// RIGHT PANEL - INTERACTIVE GUI COMPONENTS
	//=================================================================
	draw_with_backdrop("Interactive Examples", rightPanel, 70, 15, BLACK, WHITE);

	if (GuiButton(Rectangle{rightPanel, 95, 140, 30}, "Show Message Box"))
	{
		showMessageBox = true;
	}

	draw_with_backdrop("Color Picker:", rightPanel, 135, 10, BLACK, WHITE);
	GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, 6); // Smaller selector
	GuiColorPicker(Rectangle{rightPanel, 155, 120, 120}, NULL, &colorPickerValue);

	// Display selected color
	DrawRectangle(rightPanel + 30, 285, 80, 30, colorPickerValue);
	DrawRectangleLines(rightPanel + 30, 285, 80, 30, BLACK);

	// Restore original style properties
	GuiSetStyle(DEFAULT, TEXT_SIZE, defaultTextSize);
	GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, defaultColorPickerSize);

	//=================================================================
	// MODAL DIALOG HANDLING
	//=================================================================
	if (showMessageBox)
	{
		// Semi-transparent overlay
		DrawRectangle(0, 0, GameConstants::WorldWidth, GameConstants::WorldHeight, Fade(RAYWHITE, 0.8f));

		int result = GuiMessageBox(
			Rectangle{GameConstants::WorldWidth / 2 - 125, GameConstants::WorldHeight / 2 - 50, 250, 100},
			"Message Box",
			"This is an example message.\nClick OK to continue.",
			"OK");

		if (result >= 0)
		{
			showMessageBox = false;
		}
	}

	//=================================================================
	// CUSTOM MOUSE CURSOR
	//=================================================================

	// here you could use a custom cursor texture but since we're lazy we'll
	// just draw a rectangle

	int rectSize = GameConstants::WorldWidth / 40;
	DrawRectangle(virtualMousePosition.x - rectSize / 2, virtualMousePosition.y - rectSize / 2,
				  rectSize, rectSize, Fade(DARKPURPLE, 0.3f));

	// Scene transition logic
	return Scenes::NONE;
}