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
	texture = LoadTexture(AppConstants::GetAssetPath("test.png").c_str());
}

TitleScene::~TitleScene()
{
	UnloadTexture(texture);
}

Scenes TitleScene::tick(float dt)
{
	// TODO: weird that we have a Draw in Update ... Perhaps unify draw and update?
	if (GuiButton(Rectangle{10, 70, 140, 50}, "Click to start"))
	{
		return Scenes::GAME;
	}

	ClearBackground(RAYWHITE);

	auto draw_with_backdrop = [](const string &text, int x, int y, int fontSize, Color color, Color backdropColor)
	{
		DrawText(text.c_str(), x + 1, y + 1, fontSize, backdropColor);
		DrawText(text.c_str(), x, y, fontSize, color);
	};

	draw_with_backdrop("This is the Title Scene", 10, 10, 50, GOLD, BLACK);

	const int texture_x = AppConstants::ScreenWidth / 2 - texture.width;
	const int texture_y = AppConstants::ScreenHeight / 2 - texture.height;
	DrawTextureEx(texture, Vector2{(float)texture_x, (float)texture_y}, 0, 2, WHITE);

	int mouseX = GetMouseX();
	int mouseY = GetMouseY();

	int rectSize = AppConstants::ScreenWidth / 20;
	DrawRectangle(mouseX - rectSize / 2, mouseY - rectSize / 2, rectSize, rectSize, DARKPURPLE);

	DrawLine(mouseX, 0, mouseX, AppConstants::ScreenHeight, SKYBLUE);
	DrawLine(0, mouseY, AppConstants::ScreenWidth, mouseY, GREEN);

	return Scenes::NONE;
}
