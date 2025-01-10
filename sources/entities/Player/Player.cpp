#include <math.h>
#include <unordered_map>
#include <vector>
#include <math.h>

#include <raylib.h>
#include <box2d/box2d.h>

#include <LDtkLoader/World.hpp>

#include <Constants.hpp>
#include <utils/DebugUtils.hpp>

#include "Player.hpp"
#include "../../physics/PhysicsTypes.hpp"
#include "../../scenes/GameScene/GameScene.hpp"
#include "../../physics/RaycastUtils.hpp"

using namespace std;

Player::Player()
{
	this->sprite = LoadTexture(AppConstants::GetAssetPath("dinoCharactersVersion1.1/sheets/DinoSprites - vita.png").c_str());

	auto make_player_frame_rect = [](float frame_num) -> Rectangle
	{
		return {
			.x = frame_num * 24.0f,
			.y = 0.0f,
			.width = 24.0f,
			.height = 24.0f};
	};

	animation_map[IDLE] = {
		make_player_frame_rect(0),
		make_player_frame_rect(1),
		make_player_frame_rect(2),
	};

	animation_map[WALK] = {
		make_player_frame_rect(3),
		make_player_frame_rect(4),
		make_player_frame_rect(5),
	};

	animation_map[JUMP_START] = {
		make_player_frame_rect(6),
	};

	animation_map[JUMP_APEX] = {
		make_player_frame_rect(7),
	};

	animation_map[JUMP_FALL] = {
		make_player_frame_rect(8),
	};
}

Player::~Player()
{
	UnloadTexture(this->sprite);
}

void Player::update(float dt)
{
	const float horizontalDampeningFactor = 1;

	animation_ticker -= dt;
	if (animation_ticker <= 0)
	{
		animation_ticker = animation_frame_duration;
		current_anim_frame += 1;
	}

	// dampen horizontal movement
	set_velocity_x(b2Body_GetLinearVelocity(body).x * (1 - dt * horizontalDampeningFactor));

	check_if_on_floor();
	check_if_move();
	check_if_jump();

	check_if_should_respawn();

	DebugUtils::println("Player position: x:{} y:{}", b2Body_GetPosition(body).x, b2Body_GetPosition(body).y);
}

void Player::draw()
{
	auto spritePosX = (b2Body_GetPosition(body).x * GameConstants::PhysicsWorldScale) - 12;
	auto spritePosY = (b2Body_GetPosition(body).y * GameConstants::PhysicsWorldScale) - 13;

	auto current_anim_states = animation_map[anim_state];
	auto current_anim_rect = current_anim_states[current_anim_frame % current_anim_states.size()];

	if (!looking_right)
	{
		current_anim_rect.width *= -1;
	}

	DrawTexturePro(sprite,
				   current_anim_rect,
				   {spritePosX, spritePosY, 24, 24},
				   {0, 0},
				   0.0f,
				   WHITE);
}

void Player::init_for_level(const ldtk::Entity *entity)
{
	auto pos = entity->getPosition();
	pos.x += entity->getSize().x / 2;
	pos.y += entity->getSize().y / 2;

	DebugUtils::println("Setting player position to x:{} and y:{}", pos.x, pos.y);

	level_spawn_position = {(float)pos.x / GameConstants::PhysicsWorldScale,
							(float)pos.y / GameConstants::PhysicsWorldScale};

	if (B2_IS_NULL(GameScene::worldId))
	{
		DebugUtils::println("Error: worldId is null");
		return;
	}

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = level_spawn_position;
	this->body = b2CreateBody(GameScene::worldId, &bodyDef);

	if (B2_IS_NULL(this->body))
	{
		DebugUtils::println("Error: body is null");
		return;
	}

	b2Polygon dynamicBox = b2MakeBox(0.9f, 1.0f);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.friction = 10.0f;
	b2CreatePolygonShape(this->body, &shapeDef, &dynamicBox);

	DebugUtils::println("Player initialized at x:{} and y:{}", level_spawn_position.x, level_spawn_position.y);
}

void Player::set_velocity_x(float vx)
{
	b2Body_SetLinearVelocity(body, {vx, b2Body_GetLinearVelocity(body).y});
}

void Player::set_velocity_y(float vy)
{
	b2Body_SetLinearVelocity(body, {b2Body_GetLinearVelocity(body).x, vy});
}

void Player::set_velocity_xy(float vx, float vy)
{
	b2Body_SetLinearVelocity(body, {vx, vy});
}

void Player::check_if_on_floor()
{
	// first, reset whether we're touching floor
	is_touching_floor = false;

	// check left, center, and right touch points
	float x_deviations[] = {-1.0f, 0.0f, 1.0f};

	for (auto x_dev : x_deviations)
	{
		// query raylib to see if we're touching floor
		auto source = b2Body_GetPosition(body);
		source.x += x_dev;

		auto target = b2Body_GetPosition(body);
		target.x += x_dev;
		target.y += 1.1;

		is_touching_floor = RaycastCheckCollisionWithUserData(
			GameScene::worldId,
			source,
			target,
			PhysicsTypes::SolidBlock);

		if (is_touching_floor)
		{
			break;
		}
	}
}

bool Player::can_move_in_x_direction(bool moving_right)
{
	float y_deviations[] = {-1.0f, 0.0f, 1.0f};
	for (auto y_dev : y_deviations)
	{
		// query raylib to see if we're touching floor
		auto source = b2Body_GetPosition(body);
		source.y += y_dev;

		auto target = b2Body_GetPosition(body);
		target.y += y_dev;

		// check left side if necessary
		target.x += (moving_right ? 1 : -1) * 1.1;

		auto is_agains_wall = RaycastCheckCollisionWithUserData(
			GameScene::worldId,
			source,
			target,
			PhysicsTypes::SolidBlock);

		if (is_agains_wall)
		{
			return false;
		}
	}

	return true;
}

void Player::check_if_jump()
{
	if (is_touching_floor && (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE)))
	{
		set_velocity_y(-25);
	}

	if (abs(b2Body_GetLinearVelocity(body).x) > 0)
	{
		anim_state = WALK;
	}
	else
	{
		anim_state = IDLE;
	}

	if (!is_touching_floor)
	{
		auto vel = b2Body_GetLinearVelocity(body).y;
		const int jump_threshold = 5;

		if (vel > jump_threshold)
		{
			anim_state = JUMP_FALL;
		}
		else if (vel < -jump_threshold)
		{
			anim_state = JUMP_START;
		}
		else
		{
			anim_state = JUMP_APEX;
		}
	}
}

void Player::check_if_move()
{
	const auto effective_speed = 15.0f;
	if (IsKeyDown(KEY_LEFT) && can_move_in_x_direction(false))
	{
		looking_right = false;
		set_velocity_x(-effective_speed);
	}

	if (IsKeyDown(KEY_RIGHT) && can_move_in_x_direction(true))
	{
		looking_right = true;
		set_velocity_x(effective_speed);
	}
}

void Player::check_if_should_respawn()
{
	auto body_pos = b2Body_GetPosition(body);
	auto is_out_of_x = body_pos.x < 0 || body_pos.x * GameConstants::PhysicsWorldScale > GameConstants::WorldWidth;
	auto is_out_of_y = body_pos.y < 0 || body_pos.y * GameConstants::PhysicsWorldScale > GameConstants::WorldHeight;

	if (is_out_of_x || is_out_of_y)
	{
		set_velocity_xy(0, 0);
		auto currentRot = b2Body_GetRotation(body);
		b2Body_SetTransform(body, level_spawn_position, currentRot);
	}
}