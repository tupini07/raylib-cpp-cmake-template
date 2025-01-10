#pragma once

#include <box2d/box2d.h>
#include <string>

using namespace std;

struct RayCastContext
{
    b2ShapeId shapeId;
    b2Vec2 point;
    b2Vec2 normal;
    float fraction;
};

float RayCastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void *context)
{
    RayCastContext *myContext = static_cast<RayCastContext *>(context);
    myContext->shapeId = shapeId;
    myContext->point = point;
    myContext->normal = normal;
    myContext->fraction = fraction;
    return fraction;
}

RayCastContext RaycastGetFirstFixtureFromSourceToTarget(b2WorldId worldId, b2Vec2 source, b2Vec2 target)
{

    b2Vec2 translation = b2Sub(target, source);
    auto viewFilter = b2DefaultQueryFilter();
    RayCastContext context = {0};

    b2World_CastRay(worldId, source, translation, viewFilter, RayCastCallback, &context);

    return context;
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
bool RaycastCheckCollisionWithUserData(b2WorldId worldId, b2Vec2 source, b2Vec2 target, string expected_user_data)
{
    auto context = RaycastGetFirstFixtureFromSourceToTarget(worldId, source, target);
    if (B2_IS_NON_NULL(context.shapeId))
    {
        auto body = b2Shape_GetBody(context.shapeId);
        auto userData = b2Body_GetUserData(body);

        if (userData)
        {
            string body_user_data = (char *)userData;
            return body_user_data == expected_user_data;
        }
    }

    return false;
}