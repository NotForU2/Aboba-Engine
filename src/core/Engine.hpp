#pragma once
#include <entt/entt.hpp>
#include "Window.hpp"
#include "Timer.hpp"
#include "Scene.hpp"
#include "../ecs/Components.hpp"
#include "../ecs/CameraComponent.hpp"
#include "../ecs/TransformComponent.hpp"
#include "../ecs/MeshComponent.hpp"
#include "../system/InputSystem.hpp"
#include "../system/MovementSystem.hpp"
#include "../system/CollisionSystem.hpp"
#include "../graphics/VulkanRenderer.hpp"

class Engine
{
public:
  Engine();
  ~Engine();

  bool Init();
  void Run();

private:
  void ProccessInput(float dt);
  void Update(float dt);
  void Render();

  const char *mAppName = "Aboba Engine";
  const char *mEngineName = "Aboba Engine";
  bool mIsRunning;

  Window mWindow;
  VulkanContext mContext;
  VulkanRenderer mRenderer;
  AssetManager mAssetManager;
  Scene mScene;

  InputSystem mInputSystem;
  MovementSystem mMovementSystem;
  CollisionSystem mCollisionSystem;
};