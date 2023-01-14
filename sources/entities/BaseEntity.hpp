#pragma once

class BaseEntity
{
public:
    virtual ~BaseEntity() = default;
    virtual void draw() = 0;
    virtual void update(float dt) = 0;
};
