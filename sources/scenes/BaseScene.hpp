#pragma once

#include "Scenes.hpp"

class BaseScene
{
public:
    virtual ~BaseScene() = default;
    virtual Scenes tick(float dt) = 0;
};