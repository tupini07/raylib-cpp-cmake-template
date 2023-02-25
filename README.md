# Raylib C++ CMake Template

<p align="center">
    <img 
        src="https://user-images.githubusercontent.com/3422347/214886231-efaf580b-ffe6-44e0-b40b-ea8d84df3754.gif" 
        style="width: 100%; max-width: 400px;" width="400" align="center"
    />
</p>


This repo constains a small game that you can use as a template for your Raylib
games if you wish to use C++ and CMake.

CMake is configured to automatically download the dependencies of your game
through git, and pin them to a specific commit version (for example, [see how
we're importing the raylib
dependency](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/CMakeLists.txt#L25-L31)).
This provides a very flexible mechanism to include new dependencies, or update
the existing ones, without having to muck about with your system's package
manager, or worrying about linking external libraries at all. It is a bit slower
on the first build though, since CMake will need to build everything.

For the moment, the project is using the following dependencies which should be
useful in many games:

- [LDtkLoader](https://github.com/Madour/LDtkLoader) - used to load and help
  with drawing a map made with the awesome [LDtk](https://ldtk.io/).
- [box2d](https://github.com/erincatto/box2d) - ubuquitous and easy to use 2D
  physics engine.
- [fmt](https://github.com/fmtlib/fmt) - logging and string formatting library
  that makes your life much easier.

If you don't know where to start or how to use this template then check out [the wiki](https://github.com/tupini07/raylib-cpp-cmake-template/wiki)!

## Important files

You can get a good understanding of how things work by reading through the
following files:

- [[main.cpp](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/sources/main.cpp)]
  this is, of course, the entrpoint of the game. It maintains a top-level frame
  buffer to which all draw operations are done, and then this buffer is drawn to
  the screen, with a specific scale (meaning you can easily scale and/or
  translate your game without having to worry about the actual screen
  resolution). It also calls the SceneManager's update and draw methods.
- [[SceneManager](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/sources/scenes/SceneManager.hpp)]
  Implements an extremly simple scene manager. It updates and draws the current
  scene or switches to a new scene if necessary.
- [[TitleScene](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/sources/scenes/TitleScene/TitleScene.cpp)]
  This is the main screen that is shown when the game starts. It doesn't contain
  much, just a keypress listener to switch on to the actual GameScene.
- [[GameScene](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/sources/scenes/GameScene/GameScene.cpp)]
  This is the scene where the main _game_ happens. It's not really a game per
  se, just a showcase of how you would set up a
  [Player](https://github.com/tupini07/raylib-cpp-cmake-template/blob/c6996a2477e713671337e4c40d1b602f10acb01a/sources/entities/Player/Player.cpp),
  draw an LDtk map, and add some physics to everything using Box2D.


# Questions and comments

If you have any question then feel free to [create a new discussion](https://github.com/tupini07/raylib-cpp-cmake-template/discussions/new), or if you see any issue then go ahead and [open a new issue](https://github.com/tupini07/raylib-cpp-cmake-template/issues/new). 

If you see anything that can be improved then feel free to make a PR! Those are always welcome 🙂 Another welcomed contribution is that of going through the wiki and clarifying content or adding new things you think might be helpful for others.
