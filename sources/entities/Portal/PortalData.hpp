#pragma once

#include <string>
#include <box2d/box2d.h>

/**
 * Structure to hold portal destination data.
 */
struct PortalData
{
    std::string levelName; // Name of the destination level
    b2Vec2 position;       // Position to teleport to inside the destination level

    PortalData(const std::string &level, float x, float y)
        : levelName(level), position(b2Vec2(x, y)) {}
};