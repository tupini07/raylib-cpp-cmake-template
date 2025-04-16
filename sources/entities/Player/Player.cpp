#include <math.h>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <cstring>

#include <raylib.h>
#include <box2d/box2d.h>

#include <LDtkLoader/World.hpp>

#include <Constants.hpp>
#include <utils/DebugUtils.hpp>
#include <utils/Camera.hpp>

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
        return Rectangle{
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
    // Update timers
    if (jump_buffer_timer > 0)
        jump_buffer_timer -= dt;
    if (coyote_time_timer > 0)
        coyote_time_timer -= dt;

    // Check for jump input - store it in the buffer
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE))
    {
        jump_requested = true;
        jump_buffer_timer = jump_buffer_time;
    }

    // Handle animation timing
    animation_ticker -= dt;
    if (animation_ticker <= 0)
    {
        animation_ticker = animation_frame_duration;
        current_anim_frame += 1;
    }

    check_collisions();
    check_portal_collisions();
    handle_movement_states(dt);
    apply_horizontal_movement(dt);
    handle_jumping(dt);
    update_animation_state();
}

void Player::check_collisions()
{
    // Reset collision flags
    is_touching_floor = false;
    is_touching_wall_left = false;
    is_touching_wall_right = false;

    // Check floor collisions
    const float x_deviations[] = {-0.4f, 0.0f, 0.4f};
    for (auto x_dev : x_deviations)
    {
        // Query ground collision
        auto source = body->GetPosition();
        source.x += x_dev;

        auto target = body->GetPosition();
        target.x += x_dev;
        target.y += 0.6f;

        is_touching_floor = RaycastCheckCollisionWithUserData(
            GameScene::world.get(),
            source,
            target,
            PhysicsTypes::SolidBlock);

        if (is_touching_floor)
            break;
    }

    // Check left wall collision with multiple rays
    float y_deviations[] = {-0.4f, 0.0f, 0.4f};
    for (auto y_dev : y_deviations)
    {
        auto source = body->GetPosition();
        source.y += y_dev;

        auto target = body->GetPosition();
        target.y += y_dev;
        target.x -= 0.6f; // Check left side

        is_touching_wall_left = RaycastCheckCollisionWithUserData(
            GameScene::world.get(),
            source,
            target,
            PhysicsTypes::SolidBlock);

        if (is_touching_wall_left)
            break;
    }

    for (auto y_dev : y_deviations)
    {
        auto source = body->GetPosition();
        source.y += y_dev;

        auto target = body->GetPosition();
        target.y += y_dev;
        target.x += 0.6f; // Check right side

        is_touching_wall_right = RaycastCheckCollisionWithUserData(
            GameScene::world.get(),
            source,
            target,
            PhysicsTypes::SolidBlock);

        if (is_touching_wall_right)
            break;
    }

    // Update coyote time when leaving the ground
    if (is_touching_floor)
    {
        coyote_time_timer = coyote_time; // Reset coyote time when on ground
    }
}

void Player::check_portal_collisions()
{
    if (!body)
        return;

    b2AABB aabb;
    b2Vec2 position = body->GetPosition();

    float playerWidth = 0.45f;
    float playerHeight = 0.45f;

    aabb.lowerBound = b2Vec2(position.x - playerWidth, position.y - playerHeight);
    aabb.upperBound = b2Vec2(position.x + playerWidth, position.y + playerHeight);

    PortalQueryCallback callback;
    world->QueryAABB(&callback, aabb);

    is_touching_portal = callback.m_found;

    if (callback.m_found && callback.m_portalData)
    {
        // store a direct reference to the portal data so we can use it later
        currentPortal = callback.m_portalData;
    }
    else
    {
        // If we're not touching a portal, reset the current portal reference
        ResetPortalStatus();
    }
}

void Player::handle_movement_states(float dt)
{
    b2Vec2 velocity = body->GetLinearVelocity();

    // Determine movement state based on collisions and velocity
    if (is_touching_floor)
    {
        movement_state = GROUNDED;
    }
    else if (velocity.y < 0)
    {
        movement_state = JUMPING;
    }
    else
    {
        movement_state = FALLING;
    }
}

void Player::apply_horizontal_movement(float dt)
{
    b2Vec2 velocity = body->GetLinearVelocity();
    float target_x_velocity = 0.0f;

    // Handle horizontal input
    if (IsKeyDown(KEY_LEFT))
    {
        looking_right = false;
        target_x_velocity = -move_speed;
    }

    if (IsKeyDown(KEY_RIGHT))
    {
        looking_right = true;
        target_x_velocity = move_speed;
    }

    // Special case for wall collisions - don't inhibit horizontal movement when pressing against walls
    // This allows the player to naturally fall down walls rather than clinging to them
    if ((is_touching_wall_left && target_x_velocity < 0) ||
        (is_touching_wall_right && target_x_velocity > 0))
    {
        // Still allow natural damping when not pressing movement keys
        if (target_x_velocity == 0)
        {
            velocity.x *= horizontal_damping;
            apply_velocity(velocity.x, velocity.y);
        }
        // Don't apply any horizontal velocity change when pressing against wall
        // This allows the player to fall naturally down walls
    }
    else
    {
        // Normal movement when not against walls or moving away from them
        if (target_x_velocity == 0)
        {
            velocity.x *= horizontal_damping;
        }
        else
        {
            velocity.x = target_x_velocity;
        }

        apply_velocity(velocity.x, velocity.y);
    }
}

void Player::handle_jumping(float dt)
{
    b2Vec2 velocity = body->GetLinearVelocity();

    bool can_jump = (is_touching_floor || coyote_time_timer > 0);

    // Handle normal jumping
    if (jump_requested && can_jump)
    {
        velocity.y = -jump_force;
        jump_requested = false;
        jump_buffer_timer = 0;
        coyote_time_timer = 0;
        movement_state = JUMPING;
    }

    // Execute jump if buffer is active and we hit the ground
    if (jump_buffer_timer > 0 && is_touching_floor)
    {
        velocity.y = -jump_force;
        jump_buffer_timer = 0;
    }

    // Apply final velocity
    apply_velocity(velocity.x, velocity.y);

    if (velocity.y < 0)
    {
        jump_requested = false;
    }
}

void Player::apply_velocity(float vx, float vy)
{
    body->SetLinearVelocity({vx, vy});
}

void Player::draw()
{
    // First, get the player position in screen space (using the camera's physics scale)
    Vector2 playerPosScreen = GameCamera::camera.PhysicsToScreen(body->GetPosition());

    // Calculate the sprite position with offset. Center the 24x24 sprite on the
    // physics position (physics position represents the center of the sprite)
    Vector2 screenPos = {
        playerPosScreen.x - 12, // Center horizontally (half of sprite width)
        playerPosScreen.y - 12  // Center vertically (half of sprite height)
    };

    screenPos.y -= 1; // Slight upward adjustment for better visual alignment

    auto current_anim_states = animation_map[anim_state];
    auto current_anim_rect = current_anim_states[current_anim_frame % current_anim_states.size()];

    if (!looking_right)
    {
        current_anim_rect.width *= -1;
    }

    DrawTexturePro(sprite,
                   current_anim_rect,
                   {screenPos.x, screenPos.y, 24, 24},
                   {0, 0},
                   0.0f,
                   WHITE);

// Debug visualization
#ifdef DEBUG
    // Draw collision points for floor and walls
    if (is_touching_floor)
    {
        DrawCircle(playerPosScreen.x, playerPosScreen.y + 10, 2, GREEN);
    }

    if (is_touching_wall_left)
    {
        DrawCircle(playerPosScreen.x - 10, playerPosScreen.y, 2, RED);
    }

    if (is_touching_wall_right)
    {
        DrawCircle(playerPosScreen.x + 10, playerPosScreen.y, 2, RED);
    }
#endif
}

void Player::reset_physics_for_level(b2World *physicsWorld)
{
    // Store the old world reference before destroying it
    world = physicsWorld;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.allowSleep = false;

    this->body = physicsWorld->CreateBody(&bodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(0.45f, 0.45f); // Slightly smaller hitbox for better collision

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.1f;
    fixtureDef.restitution = 0.0f;

    fixture = body->CreateFixture(&fixtureDef);

    // Reset all movement state when recreating physics
    movement_state = FALLING;
    is_touching_floor = false;
    is_touching_wall_left = false;
    is_touching_wall_right = false;
    jump_requested = false;
    jump_buffer_timer = 0.0f;
    coyote_time_timer = 0.0f;

    // Reset portal status and cooldown
    is_touching_portal = false;
    currentPortal = nullptr;
}

void Player::update_animation_state()
{
    // Update animation state based on movement state and velocity
    b2Vec2 velocity = body->GetLinearVelocity();

    switch (movement_state)
    {
    case GROUNDED:
        if (abs(velocity.x) > 0.5f)
        {
            anim_state = WALK;
        }
        else
        {
            anim_state = IDLE;
        }
        break;

    case JUMPING:
        anim_state = JUMP_START;
        break;

    case FALLING:
        if (velocity.y > 5.0f)
        {
            anim_state = JUMP_FALL;
        }
        else
        {
            anim_state = JUMP_APEX;
        }
        break;
    }
}

b2Vec2 Player::GetPosition() const
{
    if (body)
    {
        return body->GetPosition();
    }
    return {0.0f, 0.0f};
}

void Player::ResetPortalStatus()
{
    is_touching_portal = false;
    currentPortal = nullptr;
}

void Player::teleport_to(const b2Vec2 &position)
{
    if (body)
    {
        DebugUtils::println("Teleporting player to physics position: {}, {}", position.x, position.y);

        body->SetLinearVelocity({0, 0});
        body->SetTransform(position, 0);

        // Calculate screen position for debugging
        Vector2 screenPos = GameCamera::camera.PhysicsToScreen(position);

        // Reset any movement state variables
        movement_state = FALLING;
        jump_requested = false;
        jump_buffer_timer = 0.0f;
        coyote_time_timer = 0.0f;
    }
}