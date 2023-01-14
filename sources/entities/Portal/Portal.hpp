#pragma once

#include <box2d/box2d.h>
#include <LDtkLoader/Entity.hpp>

class Portal
{
private:
    Texture2D sprite;
    b2Body *body{};

public:
    Portal(const ldtk::Entity *entity, b2World *physicsWorld);
    ~Portal();
};

