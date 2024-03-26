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
	set_velocity_x(body->GetLinearVelocity().x * (1 - dt * horizontalDampeningFactor));

	check_if_on_floor();
	check_if_move();
	check_if_jump();

	check_if_should_respawn();
}

void Player::draw()
{
	auto spritePosX = (body->GetPosition().x * GameConstants::PhysicsWorldScale) - 12;
	auto spritePosY = (body->GetPosition().y * GameConstants::PhysicsWorldScale) - 13;

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

void Player::init_for_level(const ldtk::Entity *entity, b2World *physicsWorld)
{
	auto pos = entity->getPosition();

	DebugUtils::println("Setting player position to x:{} and y:{}", pos.x, pos.y);

	level_spawn_position = {(float)pos.x / GameConstants::PhysicsWorldScale,
							(float)pos.y / GameConstants::PhysicsWorldScale};

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.fixedRotation = true;
	bodyDef.position.Set(level_spawn_position.x, level_spawn_position.y);

	this->body = physicsWorld->CreateBody(&bodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.9, 1);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 10.0f;

	body->CreateFixture(&fixtureDef);
}

void Player::set_velocity_x(float vx)
{
	body->SetLinearVelocity({
		vx,
		body->GetLinearVelocity().y,
	});
}

void Player::set_velocity_y(float vy)
{
	body->SetLinearVelocity({
		body->GetLinearVelocity().x,
		vy,
	});
}

void Player::set_velocity_xy(float vx, float vy)
{
	body->SetLinearVelocity({vx, vy});
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
		auto source = body->GetPosition();
		source.x += x_dev;

		auto target = body->GetPosition();
		target.x += x_dev;
		target.y += 1.1;

		is_touching_floor = RaycastCheckCollisionWithUserData(
			GameScene::world.get(),
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
		auto source = body->GetPosition();
		source.y += y_dev;

		auto target = body->GetPosition();
		target.y += y_dev;

		// check left side if necessary
		target.x += (moving_right ? 1 : -1) * 1.1;

		auto is_agains_wall = RaycastCheckCollisionWithUserData(
			GameScene::world.get(),
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

	if (abs(body->GetLinearVelocity().x) > 0)
	{
		anim_state = WALK;
	}
	else
	{
		anim_state = IDLE;
	}

	if (!is_touching_floor)
	{
		auto vel = body->GetLinearVelocity().y;
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
	auto body_pos = body->GetPosition();
	auto is_out_of_x = body_pos.x < 0 || body_pos.x * GameConstants::PhysicsWorldScale > GameConstants::WorldWidth;
	auto is_out_of_y = body_pos.y < 0 || body_pos.y * GameConstants::PhysicsWorldScale > GameConstants::WorldHeight;

	if (is_out_of_x || is_out_of_y)
	{
		set_velocity_xy(0, 0);
		body->SetTransform(level_spawn_position, 0);
	}
}