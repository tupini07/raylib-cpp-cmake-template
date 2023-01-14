#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include <raylib.h>
#include <Constants.hpp>

#include "entities/Player/Player.hpp"
#include "scenes/SceneManager.hpp"
#include "scenes/Scenes.hpp"

void UpdateDrawFrame();
RenderTexture2D frameBuffer;

int main()
{
	InitWindow(
		AppConstants::ScreenWidth,
		AppConstants::ScreenHeight,
		AppConstants::WindowTitle.c_str());

	frameBuffer = LoadRenderTexture(GameConstants::WorldWidth, GameConstants::WorldHeight);

	SceneManager::initialize();
	SceneManager::set_current_screen(Scenes::TITLE);

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
	SetTargetFPS(60); // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		UpdateDrawFrame();
	}
#endif

	SceneManager::cleanup();
	UnloadRenderTexture(frameBuffer);
	CloseWindow();
	return 0;
}

void UpdateDrawFrame()
{
	float dt = GetFrameTime();
	SceneManager::update(dt);

	if (IsKeyDown(KEY_Q))
	{
		CloseWindow();
		return;
	}

	BeginTextureMode(frameBuffer);
	ClearBackground(WHITE);

	SceneManager::draw();
	EndTextureMode();

	BeginDrawing();
	// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
	DrawTexturePro(frameBuffer.texture,
				   {0, 0, (float)frameBuffer.texture.width, -(float)frameBuffer.texture.height},
				   {0, 0, AppConstants::ScreenWidth, AppConstants::ScreenHeight},
				   {0, 0}, 0, WHITE);
	EndDrawing();
}