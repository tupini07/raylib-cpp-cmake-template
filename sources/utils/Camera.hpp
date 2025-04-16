#pragma once

#include <raylib.h>
#include <raymath.h> // Added for Clamp function
#include <box2d/box2d.h>
#include "../Constants.hpp"

class ViewportCamera
{
public:
    ViewportCamera() : physicsScale(16.0f),
                       offset({0.0f, 0.0f}),
                       zoom(1.0f),
                       worldMinX(0.0f),
                       worldMinY(0.0f),
                       worldMaxX(GameConstants::WorldWidth / 16.0f), // Default to constants (in physics units)
                       worldMaxY(GameConstants::WorldHeight / 16.0f)
    {
    }

    // Convert a physics world position to screen position
    Vector2 PhysicsToScreen(const b2Vec2 &physicsPos) const
    {
        return {
            physicsPos.x * physicsScale + offset.x,
            physicsPos.y * physicsScale + offset.y};
    }

    Vector2 PhysicsToScreen(const b2Vec2 &physicsPos, float offsetX, float offsetY) const
    {
        return {
            physicsPos.x * physicsScale + offset.x + offsetX,
            physicsPos.y * physicsScale + offset.y + offsetY};
    }

    float PhysicsToScreen(float physicsValue) const
    {
        return physicsValue * physicsScale * zoom;
    }

    b2Vec2 ScreenToPhysics(const Vector2 &screenPos) const
    {
        return {
            (screenPos.x - offset.x) / physicsScale,
            (screenPos.y - offset.y) / physicsScale};
    }

    b2Vec2 ScreenToPhysics(float screenX, float screenY) const
    {
        return {
            (screenX - offset.x) / physicsScale,
            (screenY - offset.y) / physicsScale};
    }

    float ScreenToPhysics(float screenValue) const
    {
        return screenValue / (physicsScale * zoom);
    }

    // Follow a target physics position with the camera, with bounds checking
    void FollowTargetWithBounds(const b2Vec2 &targetPhysicsPos,
                                float levelMinX, float levelMinY,
                                float levelMaxX, float levelMaxY,
                                float smoothSpeed = 0.1f)
    {
        // Convert physics position to screen space WITHOUT using current offset
        Vector2 targetScreenPos = {
            targetPhysicsPos.x * physicsScale,
            targetPhysicsPos.y * physicsScale};

        // Calculate the center of the view
        float centerX = GameConstants::WorldWidth / 2.0f;
        float centerY = GameConstants::WorldHeight / 2.0f;

        Vector2 desiredOffset = {
            centerX - targetScreenPos.x,
            centerY - targetScreenPos.y};

        ConstrainOffsetToBounds(desiredOffset);

        Vector2 newOffset = {
            Lerp(offset.x, desiredOffset.x, smoothSpeed),
            Lerp(offset.y, desiredOffset.y, smoothSpeed)};

        offset = newOffset;
    }

    // Set camera world bounds in physics units
    void SetWorldBounds(float minX, float minY, float maxX, float maxY)
    {
        worldMinX = minX;
        worldMinY = minY;
        worldMaxX = maxX;
        worldMaxY = maxY;
    }

    // Get current world bounds in physics units
    void GetWorldBounds(float &minX, float &minY, float &maxX, float &maxY) const
    {
        minX = worldMinX;
        minY = worldMinY;
        maxX = worldMaxX;
        maxY = worldMaxY;
    }

    // Follow a target physics position with the camera, using stored world bounds
    void FollowTargetWithStoredBounds(const b2Vec2 &targetPhysicsPos, float smoothSpeed = 0.1f)
    {
        FollowTargetWithBounds(targetPhysicsPos, worldMinX, worldMinY, worldMaxX, worldMaxY, smoothSpeed);
    }

    // Helper method for linear interpolation (lerp)
    float Lerp(float start, float end, float amount) const
    {
        return start + amount * (end - start);
    }

    // Reset the camera and immediately center it on a target position
    void ResetAndCenter(const b2Vec2 &targetPhysicsPos)
    {
        // Reset zoom
        zoom = 1.0f;

        Vector2 targetScreenPos = {
            targetPhysicsPos.x * physicsScale,
            targetPhysicsPos.y * physicsScale};

        float centerX = GameConstants::WorldWidth / 2.0f;
        float centerY = GameConstants::WorldHeight / 2.0f;

        Vector2 newOffset = {
            centerX - targetScreenPos.x,
            centerY - targetScreenPos.y};

        ConstrainOffsetToBounds(newOffset);

        offset = newOffset;
    }

    // Constrain an offset to the world bounds
    void ConstrainOffsetToBounds(Vector2 &offsetToConstrain) const
    {
        // Calculate level dimensions in screen space
        float levelWidthScreen = (worldMaxX - worldMinX) * physicsScale;
        float levelHeightScreen = (worldMaxY - worldMinY) * physicsScale;

        bool levelWiderThanScreen = levelWidthScreen > GameConstants::WorldWidth;
        bool levelTallerThanScreen = levelHeightScreen > GameConstants::WorldHeight;

        if (levelWiderThanScreen)
        {
            // For X-axis: constrain camera to level bounds
            float minOffsetX = GameConstants::WorldWidth - levelWidthScreen;
            float maxOffsetX = 0;
            offsetToConstrain.x = Clamp(offsetToConstrain.x, minOffsetX, maxOffsetX);
        }
        else
        {
            // If level is narrower than screen, center it
            offsetToConstrain.x = (GameConstants::WorldWidth - levelWidthScreen) / 2.0f;
        }

        if (levelTallerThanScreen)
        {
            // For Y-axis: constrain camera to level bounds
            float minOffsetY = GameConstants::WorldHeight - levelHeightScreen;
            float maxOffsetY = 0;
            offsetToConstrain.y = Clamp(offsetToConstrain.y, minOffsetY, maxOffsetY);
        }
        else
        {
            // If level is shorter than screen, center it
            offsetToConstrain.y = (GameConstants::WorldHeight - levelHeightScreen) / 2.0f;
        }
    }

    // Set the camera zoom factor
    void SetZoom(float newZoom)
    {
        if (newZoom > 0)
        {
            zoom = newZoom;
        }
    }

    // Set the physics scale factor
    void SetPhysicsScale(float newScale)
    {
        if (newScale > 0)
        {
            physicsScale = newScale;
        }
    }

    // Get the current physics scale factor
    float GetPhysicsScale() const
    {
        return physicsScale;
    }

    // Get the current camera offset
    Vector2 GetOffset() const
    {
        return offset;
    }

    // A complete reset of camera state to ensure physics and visuals realign
    void Reset()
    {
        offset = {0.0f, 0.0f};
        zoom = 1.0f;
        physicsScale = 16.0f;
        worldMinX = 0.0f;
        worldMinY = 0.0f;
        worldMaxX = GameConstants::WorldWidth / 16.0f;
        worldMaxY = GameConstants::WorldHeight / 16.0f;
    }

private:
    float physicsScale; // Physics world units to screen units conversion factor
    Vector2 offset;     // Camera offset in screen coordinates
    float zoom;         // Camera zoom factor

    // World bounds in physics units
    float worldMinX;
    float worldMinY;
    float worldMaxX;
    float worldMaxY;
};

// Global camera instance for easy access
namespace GameCamera
{
    inline ViewportCamera camera;
}