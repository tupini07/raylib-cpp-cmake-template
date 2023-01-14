#include <raylib.h>
#include <box2d/box2d.h>
#include <LDtkLoader/Project.hpp>
#include <LDtkLoader/World.hpp>
#include <fmt/core.h>

#include <Constants.hpp>
#include <utils/DebugUtils.hpp>

#include "GameScene.hpp"
#include "../../physics/PhysicsTypes.hpp"
#include "../Scenes.hpp"

#include "./entities/BaseEntity.hpp"

using namespace std;

Player *GameScene::player = nullptr;
b2World *GameScene::world = nullptr;

GameScene::GameScene()
{
	player = new Player();
	ldtkProject = new ldtk::Project();

	ldtkProject->loadFromFile(AppConstants::GetAssetPath("world.ldtk"));

	ldtkWorld = &ldtkProject->getWorld();

	current_level = -1;
	set_selected_level(0);
}

GameScene::~GameScene()
{
	delete ldtkProject;
	delete ldtkWorld;
	delete player;
	delete world;

	ldtkProject = nullptr;

	UnloadTexture(renderedLevelTexture);
	UnloadTexture(currentTilesetTexture);
}

void GameScene::draw()
{
	ClearBackground(RAYWHITE);
	DrawTextureRec(renderedLevelTexture,
				   {0, 0, (float)renderedLevelTexture.width, (float)-renderedLevelTexture.height},
				   {0, 0}, WHITE);

	player->draw();

	// DEBUG stuff
	DebugUtils::draw_physics_objects_bounding_boxes(world);
}

Scenes GameScene::update(float dt)
{
	const float timeStep = 1.0f / 60.0f;
	const int32 velocityIterations = 6;
	const int32 positionIterations = 2;

	world->Step(timeStep, velocityIterations, positionIterations);

	player->update(dt);

	return Scenes::NONE;
}

void GameScene::set_selected_level(int lvl)
{
	// unload current tileset texture if necessary
	if (current_level >= 0)
	{
		UnloadTexture(currentTilesetTexture);
	}

	if (world != nullptr)
	{
		// if we had an old world then delete it and recreate
		// a new one for the new level
		delete world;
		world = nullptr;
	}

	b2Vec2 gravity(0.0f, 60.0f);
	world = new b2World(gravity);

	current_level = lvl;

	currentLdtkLevel = &ldtkWorld->getLevel(current_level);

	DebugUtils::println("----------------------------------------------");
	DebugUtils::print("Loaded LDTK map with {}  levels in it", ldtkWorld->allLevels().size());
	DebugUtils::print("The loaded level is {} and it has {} layers", current_level, currentLdtkLevel->allLayers().size());
	for (auto &&layer : currentLdtkLevel->allLayers())
	{
		DebugUtils::print("  - {}", layer.getName());
	}

	auto testTileLayerTileset = currentLdtkLevel->getLayer("TileLayer").getTileset();

	DebugUtils::println("The path to the tile layer tileset is: {}", testTileLayerTileset.path);
	DebugUtils::println("----------------------------------------------");

	auto levelSize = currentLdtkLevel->size;
	auto renderTexture = LoadRenderTexture(levelSize.x, levelSize.y);

	BeginTextureMode(renderTexture);

	if (currentLdtkLevel->hasBgImage())
	{
		DebugUtils::println("Drawing background image");
		auto img = currentLdtkLevel->getBgImage();
		auto imgTex = LoadTexture(AppConstants::GetAssetPath(img.path.c_str()).c_str());
		SetTextureFilter(imgTex, TEXTURE_FILTER_TRILINEAR);

		// Draw texture and repeat it 5x5 times withing the specified rect
		DrawTextureQuad(imgTex, {5, 5}, {0, 0}, {0, 0, GameConstants::WorldWidth, GameConstants::WorldHeight}, WHITE);
	}

	// draw all tileset layers
	for (auto &&layer : currentLdtkLevel->allLayers())
	{
		if (layer.hasTileset())
		{
			currentTilesetTexture = LoadTexture(AppConstants::GetAssetPath(layer.getTileset().path).c_str());
			// if it is a tile layer then draw every tile to the frame buffer
			for (auto &&tile : layer.allTiles())
			{
				auto source_pos = tile.getTextureRect();
				auto tile_size = float(layer.getTileset().tile_size);

				Rectangle source_rect = {
					.x = float(source_pos.x),
					.y = float(source_pos.y),
					.width = tile.flipX ? -tile_size : tile_size,
					.height = tile.flipY ? -tile_size : tile_size,
				};

				Vector2 target_pos = {
					(float)tile.getPosition().x,
					(float)tile.getPosition().y,
				};

				DrawTextureRec(currentTilesetTexture, source_rect, target_pos, WHITE);
			}
		}
	}

	EndTextureMode();
	renderedLevelTexture = renderTexture.texture;

	// get entity positions
	DebugUtils::println("Entities in level:");
	for (auto &&entity : currentLdtkLevel->getLayer("Entities").allEntities())
	{
		DebugUtils::println("  - {}", entity.getName());
		if (entity.getName() == "Player")
		{
			player->init_for_level(&entity, world);
		}

		if (entity.getName() == "Portal")
		{
			float target_lvl = entity.getField<float>("level_destination").value();
			DebugUtils::println("Portal goes to level: {}", target_lvl);
		}
	}

	// create solid blocks on level
	DebugUtils::println("Loading solid blocks in level:");
	for (auto &&entity : currentLdtkLevel->getLayer("PhysicsEntities").allEntities())
	{
		// box2d width and height start from the center of the box
		auto b2width = entity.getSize().x / 2.0f;
		auto b2height = entity.getSize().y / 2.0f;

		auto centerX = entity.getPosition().x + b2width;
		auto centerY = entity.getPosition().y + b2height;

		b2BodyDef bodyDef;
		bodyDef.userData.pointer = (uintptr_t)PhysicsTypes::SolidBlock.c_str();
		bodyDef.position.Set(centerX / GameConstants::PhysicsWorldScale,
							 centerY / GameConstants::PhysicsWorldScale);

		b2Body *body = world->CreateBody(&bodyDef);

		b2PolygonShape groundBox;
		groundBox.SetAsBox(b2width / GameConstants::PhysicsWorldScale,
						   b2height / GameConstants::PhysicsWorldScale);

		body->CreateFixture(&groundBox, 0.0f);

		DebugUtils::println("  - x:{} y:{} width:{} height:{}",
							centerX,
							centerY,
							b2width,
							b2height);
	}
}