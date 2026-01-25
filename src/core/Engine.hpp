#pragma once
#include <entt/entt.hpp>
#include <SDL2/SDL.h>
#include <memory>
#include "Window.hpp"
#include "Timer.hpp"
#include "../ecs/Components.hpp"
#include "../system/InputSystem.hpp"
#include "../system/MovementSystem.hpp"

class Engine
{
public:
  Engine();
  ~Engine();

  bool Initialize();
  void Run();

private:
  void ProccessInput();
  void Update(float dt);
  void Render();

  entt::registry mRegistry;
  InputSystem mInputSystem;
  MovementSystem mMovementSystem;
  std::unique_ptr<Window> mWindow;
  bool mIsRunning;
};