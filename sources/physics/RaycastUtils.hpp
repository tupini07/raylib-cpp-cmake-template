#pragma once

#include <box2d/box2d.h>
#include <string>

using namespace std;

class RaysCastGetNearestCallback : public b2RayCastCallback
{
public:
    RaysCastGetNearestCallback() : m_fixture(NULL)
    {
    }

    float ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction)
    {
        m_fixture = fixture;
        m_point = point;
        m_normal = normal;
        m_fraction = fraction;
        return fraction;
    }

    b2Fixture *m_fixture;
    b2Vec2 m_point;
    b2Vec2 m_normal;
    float m_fraction;
};

b2Fixture *RaycastGetFirstFixtureFromSourceToTarget(b2World *world, b2Vec2 source, b2Vec2 target)
{
    // query raylib to see if we're touching floor
    RaysCastGetNearestCallback raycastCallback;

    GameScene::world->RayCast(&raycastCallback,
                              source,
                              target);

    return raycastCallback.m_fixture;
}

/**
 * Tries to get a collision, via raycast, that goes from the source to the target point. If there is a collision
 * then it checks if the body we detected has the specified `expected_user_data`
 *
 * @param world
 * @param source
 * @param target
 * @param user_data
 * @return true
 * @return false
 */
bool RaycastCheckCollisionWithUserData(b2World *world, b2Vec2 source, b2Vec2 target, string expected_user_data)
{
    auto fixture = RaycastGetFirstFixtureFromSourceToTarget(world, source, target);
    if (fixture)
    {
        auto collision_body = fixture->GetBody();

        if (collision_body->GetUserData().pointer)
        {
            string body_user_data = (char *)collision_body->GetUserData().pointer;
            return body_user_data == expected_user_data;
        }
    }

    return false;
}