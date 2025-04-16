#pragma once

#include <string>
#include <iostream>

#include <raylib.h>
#include <box2d/box2d.h>
#include <fmt/core.h>

#include <Constants.hpp>
#include "Camera.hpp"

using namespace std;

namespace DebugUtils
{
    inline void draw_physics_objects_bounding_boxes(const b2World *world)
    {
#ifdef DEBUG
        auto currentBody = world->GetBodyList();
        while (currentBody != nullptr)
        {
            auto pos = currentBody->GetPosition();
            Vector2 screenPos = GameCamera::camera.PhysicsToScreen(pos);
            DrawCircle(screenPos.x, screenPos.y, 2, PURPLE);

            auto fixture = currentBody->GetFixtureList();
            while (fixture != nullptr)
            {
                auto shape = fixture->GetShape();
                // Note, right now supposing all shapes are polygons, use to determine shape->GetType();

                auto polygonShape = (b2PolygonShape *)shape;
                int vertexCount = 4; // since we're assuming they're squares
                for (int j = 0; j < vertexCount; j++)
                {
                    b2Vec2 vertexA = polygonShape->m_vertices[j];

                    int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); // Get next vertex or first to close the shape
                    b2Vec2 vertexB = polygonShape->m_vertices[jj];

                    Vector2 pointA = GameCamera::camera.PhysicsToScreen({pos.x + vertexA.x, pos.y + vertexA.y});
                    Vector2 pointB = GameCamera::camera.PhysicsToScreen({pos.x + vertexB.x, pos.y + vertexB.y});

                    DrawLineV(pointA, pointB, GREEN); // Draw a line between two vertex positions
                }

                fixture = fixture->GetNext();
            }

            currentBody = currentBody->GetNext();
        }

#endif
    }

    template <typename... T>
    inline void print(fmt::format_string<T...> fmt, T &&...args)
    {
#ifdef DEBUG
        fmt::print(fmt::runtime(fmt), args...);
#endif
    }

    template <typename... T>
    inline void println(fmt::format_string<T...> fmt, T &&...args)
    {
#ifdef DEBUG
        auto formatted = fmt::format(fmt::runtime(fmt), args...);
        cout << formatted << endl;
#endif
    }
}