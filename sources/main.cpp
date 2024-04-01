#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define RAYGUI_IMPLEMENTATION

#include <raylib.h>
#include <raygui.h>

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

	GuiLoadStyleDefault();

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

	if (IsKeyDown(KEY_Q))
	{
		CloseWindow();
		return;
	}

	BeginDrawing();
	SceneManager::tick(dt);
	EndDrawing();

	//BeginTextureMode(frameBuffer);
	//ClearBackground(WHITE);

	//SceneManager::draw();

	//EndTextureMode();

	//BeginDrawing();
	//// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
	//DrawTexturePro(frameBuffer.texture,
	//	Rectangle{ 0, 0, (float)frameBuffer.texture.width, -(float)frameBuffer.texture.height },
	//	Rectangle{ 0, 0, AppConstants::ScreenWidth, AppConstants::ScreenHeight },
	//	Vector2{ 0, 0 },
	//	0, WHITE);
	//EndDrawing();
}