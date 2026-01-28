#pragma once
#include <entt/entt.hpp>
#include <SDL2/SDL.h>
#include "Window.hpp"
#include "Timer.hpp"
#include "../ecs/Components.hpp"
#include "../system/InputSystem.hpp"
#include "../system/MovementSystem.hpp"
#include "../system/RenderSystem.hpp"
#include "../system/CollisionSystem.hpp"

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
  Window mWindow;
  bool mIsRunning;

  InputSystem mInputSystem;
  MovementSystem mMovementSystem;
  RenderSystem mRenderSystem;
  CollisionSystem mCollisionSystem;
};