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

	Scenes tick(float dt) override;
};
