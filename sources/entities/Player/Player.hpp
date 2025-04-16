#pragma once

#include "../BaseEntity.hpp"
#include "../Portal/PortalData.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include <raylib.h>
#include <box2d/box2d.h>
#include <LDtkLoader/Entity.hpp>
#include <utils/DebugUtils.hpp>

using namespace std;

enum PlayerAnimationState
{
    IDLE,
    WALK,
    JUMP_START,
    JUMP_APEX,
    JUMP_FALL
};

enum PlayerMovementState
{
    GROUNDED,
    JUMPING,
    FALLING,
};

// Portal query callback for Box2D
class PortalQueryCallback : public b2QueryCallback
{
public:
    bool m_found = false;
    PortalData *m_portalData = nullptr;

    bool ReportFixture(b2Fixture *fixture) override
    {
        // Skip fixtures without user data
        if (!fixture->GetUserData().pointer)
        {
            return true; // Continue searching
        }

        // Check if this is a portal fixture by inspecting its type
        const char *fixtureType = reinterpret_cast<const char *>(fixture->GetUserData().pointer);
        if (strcmp(fixtureType, "PortalEntry") != 0)
        {
            return true; // Not a portal, continue searching
        }

        // This is a portal fixture - now get the portal data from the body
        b2Body *portalBody = fixture->GetBody();
        if (!portalBody->GetUserData().pointer)
        {
            DebugUtils::println("ERROR: Portal body has no user data!");
            return true; // Continue searching
        }

        // Get portal data from the body and store it
        m_portalData = reinterpret_cast<PortalData *>(portalBody->GetUserData().pointer);
        m_found = true;

        // Log that we found a portal
        DebugUtils::println("Portal detected! Destination level: '{}' at position: {},{}",
                            m_portalData->levelName, m_portalData->position.x, m_portalData->position.y);

        // We found a portal, stop searching
        return false;
    }
};

class Player
{
private:
    Texture2D sprite;
    b2Body *body = nullptr;
    b2World *world = nullptr;
    b2Fixture *fixture = nullptr;
    b2Vec2 level_spawn_position = {0, 0};

    // Movement state management
    PlayerMovementState movement_state = FALLING;
    bool is_touching_floor = false;
    bool is_touching_wall_left = false;
    bool is_touching_wall_right = false;
    bool looking_right = true;
    bool jump_requested = false;
    float jump_buffer_timer = 0.0f;
    float coyote_time_timer = 0.0f;

    // Portal interaction
    bool is_touching_portal = false;
    PortalData *currentPortal = nullptr;

    // Physics constants
    const float jump_force = 25.0f;
    const float move_speed = 10.0f;
    const float horizontal_damping = 0.9f;
    const float wall_slide_speed = 3.0f;
    const float jump_buffer_time = 0.15f; // Time window where jump input is remembered
    const float coyote_time = 0.15f;      // Time window where player can still jump after leaving ground

    // Animation constants
    const float animation_frame_duration = 0.2f;
    float animation_ticker = animation_frame_duration;
    size_t current_anim_frame = 0;
    PlayerAnimationState anim_state = PlayerAnimationState::IDLE;
    unordered_map<PlayerAnimationState, vector<Rectangle>> animation_map;

    // Helper methods
    void apply_velocity(float vx, float vy);
    void apply_horizontal_movement(float dt);
    void handle_jumping(float dt);
    void check_collisions();
    void check_portal_collisions();
    void handle_movement_states(float dt);
    void update_animation_state();

public:
    Player();
    ~Player();

    bool hasBeenInitialized() const { return body != nullptr; }

    void update(float dt);
    void draw();
    void reset_physics_for_level(b2World *physicsWorld);

    b2Vec2 GetPosition() const;

    bool IsTouchingPortal() const { return is_touching_portal || currentPortal != nullptr; }
    void ResetPortalStatus();

    PortalData *GetCurrentPortal() const { return currentPortal; }
    void teleport_to(const b2Vec2 &position);
};
