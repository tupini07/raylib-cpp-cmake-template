#pragma once

#include <string>

using namespace std;

const int ScreenScale = 2;
namespace GameConstants
{
    const int WorldWidth = 400;
    const int WorldHeight = 400;

    const int CellSize = 16;
}

namespace AppConstants
{
    const string WindowTitle = "Window Title";

    const int ScreenWidth = GameConstants::WorldWidth * ScreenScale;
    const int ScreenHeight = GameConstants::WorldHeight * ScreenScale;

    inline string GetAssetPath(string assetName)
    {
        return ASSETS_PATH "" + assetName;
    }
}
