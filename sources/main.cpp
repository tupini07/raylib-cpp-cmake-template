#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define RAYGUI_IMPLEMENTATION

#include <raylib.h>
#include <raygui.h>

#include <Constants.hpp>
#include "utils/Camera.hpp"

#include "entities/Player/Player.hpp"
#include "scenes/SceneManager.hpp"
#include "scenes/Scenes.hpp"

void UpdateDrawFrame();
RenderTexture2D gameRenderTexture; // Render texture for the game world

int main()
{
	InitWindow(
		AppConstants::ScreenWidth,
		AppConstants::ScreenHeight,
		AppConstants::WindowTitle.c_str());

	GuiLoadStyleDefault();

	// Create render texture at game resolution (not screen resolution)
	gameRenderTexture = LoadRenderTexture(GameConstants::WorldWidth, GameConstants::WorldHeight);
	
	// Initialize the camera with the physics scale factor (16.0f / ScreenScale)
	GameCamera::camera.SetPhysicsScale(16.0f);

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
	UnloadRenderTexture(gameRenderTexture);
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

	BeginTextureMode(gameRenderTexture);
	ClearBackground(RAYWHITE);
	
	SceneManager::tick(dt);
	
	EndTextureMode();

	BeginDrawing();
	ClearBackground(BLACK);
	
	Rectangle source = { 0.0f, 0.0f, (float)gameRenderTexture.texture.width, (float)-gameRenderTexture.texture.height };
	Rectangle dest = { 0.0f, 0.0f, AppConstants::ScreenWidth, AppConstants::ScreenHeight };
	DrawTexturePro(gameRenderTexture.texture, source, dest, Vector2{ 0, 0 }, 0.0f, WHITE);
	
	EndDrawing();
}