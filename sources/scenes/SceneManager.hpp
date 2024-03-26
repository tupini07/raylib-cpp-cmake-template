#pragma once

#include "BaseScene.hpp"
#include "TitleScene/TitleScene.hpp"
#include "GameScene/GameScene.hpp"
#include "Scenes.hpp"

#include <memory>

class SceneManager
{
private:
	static std::unique_ptr<BaseScene> current_screen;

public:
	static void set_current_screen(Scenes screen);
	static void initialize();

	static void update(float dt);
	static void draw();
	static void cleanup();
};

std::unique_ptr<BaseScene> SceneManager::current_screen = nullptr;

void SceneManager::initialize()
{
	SceneManager::set_current_screen(UNSET);
}

void SceneManager::set_current_screen(Scenes screen)
{
	if (screen == NONE)
	{
		return;
	}

	SceneManager::current_screen.reset();

	switch (screen)
	{
	case UNSET:
		SceneManager::current_screen = nullptr;
		break;
	case TITLE:
		SceneManager::current_screen = std::make_unique<TitleScene>();
		break;
	case GAME:
		SceneManager::current_screen = std::make_unique<GameScene>();
		break;
	case NONE:
		std::cerr << "Landed in NONE case for switch. This should never happen!" << std::endl;
		exit(1);
	}
}

void SceneManager::update(float dt)
{
	if (SceneManager::current_screen != nullptr)
	{
		Scenes result = SceneManager::current_screen->update(dt);
		if (result != NONE)
		{
			SceneManager::set_current_screen(result);
		}
	}
}

void SceneManager::draw()
{
	if (SceneManager::current_screen != nullptr)
	{
		SceneManager::current_screen->draw();
	}
}

void SceneManager::cleanup()
{
	if (SceneManager::current_screen != nullptr)
	{
		SceneManager::current_screen = nullptr;
	}
}