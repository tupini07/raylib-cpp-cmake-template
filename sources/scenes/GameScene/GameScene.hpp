#pragma once

#include <memory>
#include <box2d/box2d.h>
#include <raylib.h>
#include <LDtkLoader/Project.hpp>
#include <LDtkLoader/World.hpp>

#include "../BaseScene.hpp"
#include "../Scenes.hpp"

#include "../../entities/Player/Player.hpp"
#include "./entities/BaseEntity.hpp"

class GameScene : public BaseScene
{
private:
    int current_level;

    std::unique_ptr<ldtk::Project> ldtkProject;
    const ldtk::World *ldtkWorld{};
    const ldtk::Level *currentLdtkLevel{};

    Texture2D currentTilesetTexture;
    Texture2D renderedLevelTexture;

public:
    GameScene();
    ~GameScene();

    static b2WorldId worldId;
    static std::unique_ptr<Player> player;

    Scenes tick(float dt) override;

    void set_selected_level(int lvl);
};
