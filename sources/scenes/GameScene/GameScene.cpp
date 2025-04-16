#include <memory>
#include <raylib.h>
#include <box2d/box2d.h>
#include <LDtkLoader/Project.hpp>
#include <LDtkLoader/World.hpp>
#include <fmt/core.h>
#include <vector>

#include <Constants.hpp>
#include <utils/DebugUtils.hpp>
#include <utils/Camera.hpp>

#include "GameScene.hpp"
#include "../../physics/PhysicsTypes.hpp"
#include "../Scenes.hpp"
#include "../../entities/Portal/PortalData.hpp"

#include "./entities/BaseEntity.hpp"

using namespace std;

std::unique_ptr<Player> GameScene::player = nullptr;
std::unique_ptr<b2World> GameScene::world = nullptr;

// Container for allocated portal data
static std::vector<std::unique_ptr<PortalData>> portalDataList;

GameScene::GameScene()
{
	player = std::make_unique<Player>();
	ldtkProject = std::make_unique<ldtk::Project>();

	ldtkProject->loadFromFile(AppConstants::GetAssetPath("world.ldtk"));

	ldtkWorld = &ldtkProject->getWorld();

	set_selected_level("Level_0"); // default
}

GameScene::~GameScene()
{
	UnloadTexture(renderedLevelTexture);
	UnloadTexture(currentTilesetTexture);

	// Unload both textures if they exist
	if (hasBackground)
	{
		UnloadTexture(backgroundTexture);
	}
}

Scenes GameScene::tick(float dt)
{
	const float timeStep = 1.0f / 60.0f;
	const int32 velocityIterations = 6;
	const int32 positionIterations = 2;

	world->Step(timeStep, velocityIterations, positionIterations);
	player->update(dt);

	// Check if player is touching a portal and if so, teleport to the destination portal
	if (player->IsTouchingPortal())
	{
		const PortalData *portalData = player->GetCurrentPortal();

		if (portalData)
		{
			player->ResetPortalStatus();

			// Store the destination position to set after level change
			b2Vec2 savedDestinationPos = portalData->position;

			set_selected_level(portalData->levelName);

			player->teleport_to(savedDestinationPos);
			GameCamera::camera.ResetAndCenter(player->GetPosition());

			return Scenes::NONE;
		}
	}

	GameCamera::camera.FollowTargetWithStoredBounds(
		player->GetPosition(),
		0.05f // Smoothing factor
	);

	ClearBackground(SKYBLUE);

	// Get camera offset for drawing
	Vector2 cameraOffset = GameCamera::camera.GetOffset();

	// First draw the background behind everything
	if (hasBackground)
	{
		// Get the full level dimensions
		auto levelSize = currentLdtkLevel->size;

		// Calculate the number of tiles needed to cover the entire level (plus some extra)
		int tilesX = (levelSize.x / backgroundTexture.width) + 4; // Add extra tiles for safety
		int tilesY = (levelSize.y / backgroundTexture.height) + 4;

		// Calculate the start position for tiling based on camera position
		float parallaxFactor = 0.3f;
		float startX = fmodf(cameraOffset.x * parallaxFactor, (float)backgroundTexture.width) - backgroundTexture.width;
		float startY = fmodf(cameraOffset.y * parallaxFactor, (float)backgroundTexture.height) - backgroundTexture.height;

		// Draw enough background tiles to cover the visible area and beyond
		for (int y = 0; y < tilesY; y++)
		{
			for (int x = 0; x < tilesX; x++)
			{
				float tileX = startX + (x * backgroundTexture.width);
				float tileY = startY + (y * backgroundTexture.height);

				// Only draw tiles that might be visible
				if (tileX >= -backgroundTexture.width &&
					tileX <= GameConstants::WorldWidth &&
					tileY >= -backgroundTexture.height &&
					tileY <= GameConstants::WorldHeight)
				{
					DrawTexture(backgroundTexture, tileX, tileY, WHITE);
				}
			}
		}
	}

	// Then draw the actual level texture (this was computed in the set_selected_level method)
	DrawTextureRec(renderedLevelTexture,
				   {0, 0, (float)renderedLevelTexture.width, (float)-renderedLevelTexture.height},
				   cameraOffset, WHITE);

	player->draw();

	DebugUtils::draw_physics_objects_bounding_boxes(world.get());

	return Scenes::NONE;
}

void GameScene::set_selected_level(string lvl_name)
{
	// Clear the portal data list when switching levels to prevent memory leaks
	portalDataList.clear();

	// unload current tileset texture if necessary
	if (currentLdtkLevel != nullptr)
	{
		UnloadTexture(currentTilesetTexture);
	}

	// Unload textures if needed
	if (hasBackground)
	{
		UnloadTexture(backgroundTexture);
		hasBackground = false;
	}

	if (world != nullptr)
	{
		// if we had an old world then delete it and recreate a new one for the
		// new level
		world = nullptr;
	}

	b2Vec2 gravity(0.0f, 60.0f);
	world = std::make_unique<b2World>(gravity);

	currentLdtkLevel = &ldtkWorld->getLevel(lvl_name);

	DebugUtils::println("----------------------------------------------");
	DebugUtils::println("Loaded LDTK map with {}  levels in it", ldtkWorld->allLevels().size());
	DebugUtils::println("The loaded level is {} and it has {} layers", currentLdtkLevel->name, currentLdtkLevel->allLayers().size());
	for (auto &&layer : currentLdtkLevel->allLayers())
	{
		DebugUtils::print("  - {}", layer.getName());
	}

	DebugUtils::println("----------------------------------------------");

	auto levelSize = currentLdtkLevel->size;
	auto renderTexture = LoadRenderTexture(levelSize.x, levelSize.y);

	GameCamera::camera.Reset();

	// Set camera world bounds based on level size
	GameCamera::camera.SetWorldBounds(0, 0,
									  levelSize.x / GameCamera::camera.GetPhysicsScale(),
									  levelSize.y / GameCamera::camera.GetPhysicsScale());

	BeginTextureMode(renderTexture);
	// Clear with a transparent color instead of solid white
	// This allows background to show through any non-tile areas
	ClearBackground(BLANK);

	// Instead of drawing the background to the render texture, we keep it
	// separate so we can draw it before the level in each frame. This allows
	// for parallax effects and other visual tricks.
	if (currentLdtkLevel->hasBgImage())
	{
		auto backgroundPath = currentLdtkLevel->getBgImage();
		auto fullPath = AppConstants::GetAssetPath(backgroundPath.path.c_str());

		DebugUtils::println("Loading background image from path: {}", fullPath);

		// Store the background as a class member so we can draw it in the tick method
		backgroundTexture = LoadTexture(fullPath.c_str());

		// Check if texture was loaded correctly
		if (backgroundTexture.id > 0)
		{
			DebugUtils::println("Background texture loaded successfully - Width: {}, Height: {}",
								backgroundTexture.width, backgroundTexture.height);

			SetTextureFilter(backgroundTexture, TEXTURE_FILTER_TRILINEAR);
			hasBackground = true;
		}
		else
		{
			DebugUtils::println("ERROR: Failed to load background texture!");
			hasBackground = false;
		}
	}
	else
	{
		// No background in this level
		hasBackground = false;
	}

	// draw all tileset layers to the render texture. We basically 'bake' the
	// level into a single texture that we can then draw in the main loop This
	// is done as an optimization, as drawing a single precomputed texture is
	// much faster than drawing each tile separately in the main loop
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

	// Process all entities
	DebugUtils::println("Entities in level:");

	auto playerAlreadyInitialized = player->hasBeenInitialized();

	player->reset_physics_for_level(world.get()); // this will implicitly initialize the player as well

	// we only initialize the player once
	if (!playerAlreadyInitialized)
	{
		// First, locate the player entity to initialize it first
		for (auto &&entity : currentLdtkLevel->getLayer("Entities").allEntities())
		{
			if (entity.getName() == "Player")
			{

				auto pos = entity.getPosition();
				DebugUtils::println("Setting player position to x:{} and y:{}", pos.x, pos.y);
				auto level_spawn_position = GameCamera::camera.ScreenToPhysics(pos.x, pos.y);
				player->teleport_to(level_spawn_position);

				// Set level dimensions on the player for proper respawn checking
				DebugUtils::println("  - {}", entity.getName());
				break;
			}
		}
	}

	// Then process other entities, including portals
	for (auto &&entity : currentLdtkLevel->getLayer("Entities").allEntities())
	{
		if (entity.getName() != "Player")
		{
			DebugUtils::println("  - {}", entity.getName());

			// Create physics bodies for portals
			if (entity.getName() == "PortalEntry")
			{
				// Create a sensor body for the portal entry
				b2BodyDef portalBodyDef;
				portalBodyDef.type = b2_staticBody;

				// Position is at the center of the portal
				auto pos = entity.getPosition();
				auto size = entity.getSize();
				portalBodyDef.position = GameCamera::camera.ScreenToPhysics(
					pos.x + size.x / 2.0f,
					pos.y + size.y / 2.0f);

				// Check if the portal has a destination reference
				try
				{
					auto destField = entity.getField<ldtk::FieldType::EntityRef>("destination");
					if (!destField.is_null())
					{
						auto destRef = destField.value();

						try
						{
							// Get the destination level directly using the level IID from the reference
							const auto *destLevel = &ldtkWorld->getLevel(destRef.level_iid);
							std::string destLevelName = destLevel->name;

							auto destPos = destRef->getPosition();
							auto destSize = destRef->getSize();

							// Calculate center position of destination (the
							// center of the rectangle that represents the
							// destination portal)
							float destX = destPos.x + destSize.x / 2.0f;
							float destY = destPos.y + destSize.y / 2.0f;

							b2Vec2 physicsDestPos = GameCamera::camera.ScreenToPhysics(destX, destY);

							auto portalData = std::make_unique<PortalData>(
								destLevelName,
								physicsDestPos.x,
								physicsDestPos.y);

							// Store the pointer to the PortalData as user data in Box2D body
							portalBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(portalData.get());

							// Add the unique_ptr to our container to manage its lifetime
							portalDataList.push_back(std::move(portalData));

							DebugUtils::println("Portal created: Entry at ({},{}) points to exit in level: {} at position: {},{}",
												portalBodyDef.position.x, portalBodyDef.position.y,
												destLevelName, physicsDestPos.x, physicsDestPos.y);
						}
						catch (const std::exception &e)
						{
							DebugUtils::println("ERROR: Could not resolve portal destination: {}", e.what());
							continue;
						}
					}
					else
					{
						DebugUtils::println("WARNING: Portal entry has no destination set!");
						continue;
					}
				}
				catch (const std::exception &e)
				{
					DebugUtils::println("ERROR: Failed to get destination field: {}", e.what());
					continue;
				}

				b2Body *portalBody = world->CreateBody(&portalBodyDef);

				// Create a rectangle fixture for the portal
				b2PolygonShape portalBox;
				portalBox.SetAsBox(
					GameCamera::camera.ScreenToPhysics(size.x / 2.0f),
					GameCamera::camera.ScreenToPhysics(size.y / 2.0f));

				b2FixtureDef portalFixtureDef;
				portalFixtureDef.shape = &portalBox;
				portalFixtureDef.isSensor = true; // Make it a sensor so it doesn't collide physically

				// Store a persistent string for the portal type identifier
				static const char *portalTypeStr = "PortalEntry";
				portalFixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(portalTypeStr);

				portalBody->CreateFixture(&portalFixtureDef);
			}
		}
	}

	// Create solid blocks for the level
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
		bodyDef.position = GameCamera::camera.ScreenToPhysics(centerX, centerY);

		b2Body *body = world->CreateBody(&bodyDef);

		b2PolygonShape groundBox;
		groundBox.SetAsBox(GameCamera::camera.ScreenToPhysics(b2width),
						   GameCamera::camera.ScreenToPhysics(b2height));

		body->CreateFixture(&groundBox, 0.0f);

		DebugUtils::println("  - x:{} y:{} width:{} height:{}",
							centerX,
							centerY,
							b2width,
							b2height);
	}

	GameCamera::camera.ResetAndCenter(player->GetPosition());
}
