#pragma once
#include <raylib.h>

#include "../BaseScene.hpp"

class TitleScene : public BaseScene
{
private:
	Texture2D texture;

public:
	TitleScene();
	~TitleScene();

	void draw() override;
	Scenes update(float dt) override;
};
