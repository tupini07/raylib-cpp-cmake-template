#pragma once

#include "Scenes.hpp"

class BaseScene
{
public:
    virtual ~BaseScene() = default;
    virtual void draw() = 0;
    virtual Scenes update(float dt) = 0;
};