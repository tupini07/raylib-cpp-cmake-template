#include <memory>
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

std::unique_ptr<Player> GameScene::player = nullptr;
b2WorldId GameScene::worldId = b2_nullWorldId;

GameScene::GameScene()
{
	player = std::make_unique<Player>();
	ldtkProject = std::make_unique<ldtk::Project>();

	ldtkProject->loadFromFile(AppConstants::GetAssetPath("world.ldtk"));

	ldtkWorld = &ldtkProject->getWorld();

	current_level = -1;
	set_selected_level(0);
}

GameScene::~GameScene()
{
	UnloadTexture(renderedLevelTexture);
	UnloadTexture(currentTilesetTexture);

	if (B2_IS_NON_NULL(worldId))
	{
		b2DestroyWorld(worldId);
	}
}

Scenes GameScene::tick(float dt)
{
	const float timeStep = 1.0f / 60.0f;
	const int subStepCount = 4;
	;

	// world->Step(timeStep, velocityIterations, positionIterations);
	if (B2_IS_NON_NULL(worldId))
	{
		b2World_Step(worldId, timeStep, subStepCount);
	}
	player->update(dt);

	ClearBackground(RAYWHITE);
	DrawTextureRec(renderedLevelTexture,
				   {0, 0, (float)renderedLevelTexture.width, (float)-renderedLevelTexture.height},
				   {0, 0}, WHITE);

	player->draw();

	// DEBUG stuff
	DebugUtils::draw_physics_objects_bounding_boxes(worldId);

	return Scenes::NONE;
}

void GameScene::set_selected_level(int lvl)
{
	// unload current tileset texture if necessary
	if (current_level >= 0)
	{
		UnloadTexture(currentTilesetTexture);
	}

	if (B2_IS_NON_NULL(worldId))
	{
		b2DestroyWorld(worldId);
		// if we had an old world then delete it and recreate
		// a new one for the new level
		worldId = b2_nullWorldId;
	}

	auto worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, 60.0f};
	worldId = b2CreateWorld(&worldDef);

	current_level = lvl;

	currentLdtkLevel = &ldtkWorld->getLevel(current_level);

	DebugUtils::println("----------------------------------------------");
	DebugUtils::println("Loaded LDTK map with {}  levels in it", ldtkWorld->allLevels().size());
	DebugUtils::println("The loaded level is {} and it has {} layers", current_level, currentLdtkLevel->allLayers().size());
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
		auto backgroundPath = currentLdtkLevel->getBgImage();
		auto backgroundTexture = LoadTexture(AppConstants::GetAssetPath(backgroundPath.path.c_str()).c_str());
		SetTextureFilter(backgroundTexture, TEXTURE_FILTER_TRILINEAR);

		// tile background texture to cover the whole frame buffer
		for (int i = 0; i <= (GameConstants::WorldWidth / backgroundTexture.width); i++)
		{
			for (int j = 0; j <= (GameConstants::WorldHeight / backgroundTexture.height); j++)
			{
				DrawTextureV(backgroundTexture, {float(i * backgroundTexture.width), float(j * backgroundTexture.height)}, WHITE);
			}
		}
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
			player->init_for_level(&entity);
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

		auto bodyDef = b2DefaultBodyDef();
		bodyDef.position = {centerX / GameConstants::PhysicsWorldScale,
							centerY / GameConstants::PhysicsWorldScale};
		// bodyDef.fixedRotation = true;
		bodyDef.userData = (void *)PhysicsTypes::SolidBlock.c_str();
		auto bodyId = b2CreateBody(worldId, &bodyDef);

		if (B2_IS_NULL(bodyId))
		{
			DebugUtils::println("Error: body is null");
			return;
		}

		b2Polygon dynamicBox = b2MakeBox(b2width / GameConstants::PhysicsWorldScale,
										 b2height / GameConstants::PhysicsWorldScale);

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.3f;
		b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

		DebugUtils::println("  - x:{} y:{} width:{} height:{}",
							centerX,
							centerY,
							b2width,
							b2height);
	}

	DebugUtils::println("Level loaded");
}
