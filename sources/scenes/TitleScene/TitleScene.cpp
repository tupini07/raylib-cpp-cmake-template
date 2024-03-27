#include <string>

#include <raylib.h>

#include <Constants.hpp>
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


Scenes TitleScene::update(float dt)
{
	if (IsKeyPressed(KEY_C))
	{
		return Scenes::GAME;
	}

	return Scenes::NONE;
}

void TitleScene::draw()
{
	ClearBackground(RAYWHITE);

	auto draw_with_backdrop = [](const string& text, int x, int y, int fontSize, Color color, Color backdropColor) {
		DrawText(text.c_str(), x + 1, y + 1, fontSize, backdropColor);
		DrawText(text.c_str(), x, y, fontSize, color);
	};

	draw_with_backdrop("Press 'c' to play!", 10, 10, 30, GOLD, BLACK);


	const int texture_x = GameConstants::WorldWidth / 2 - texture.width / 2;
	const int texture_y = GameConstants::WorldHeight / 2 - texture.height / 2;
	DrawTexture(texture, texture_x, texture_y, WHITE);

	const string text = "This is the Title Scene";
	const int text_width = MeasureText(text.c_str(), 20);
	DrawText(text.c_str(), GameConstants::WorldWidth / 2 - text_width / 2, texture_y + texture.height + 20, 20, BLACK);

	int mouseX = GetMouseX() / ScreenScale;
	int mouseY = GetMouseY() / ScreenScale;

	int rectSize = GameConstants::WorldWidth / 20;
	DrawRectangle(mouseX - rectSize / 2, mouseY - rectSize / 2, rectSize, rectSize, DARKPURPLE);

	DrawLine(mouseX, 0, mouseX, GameConstants::WorldHeight, SKYBLUE);
	DrawLine(0, mouseY, GameConstants::WorldWidth, mouseY, GREEN);
}

