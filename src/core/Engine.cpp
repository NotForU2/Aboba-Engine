#include "Engine.hpp"

Engine::Engine() : mIsRunning(false) {}

Engine::~Engine()
{
  mVulkanRenderer.Cleanup();
  mWindow.Cleanup();
}

bool Engine::Init()
{
  try
  {
    mWindow.Init(mAppName, 800, 600);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Window Init Error: " << e.what() << std::endl;
    return false;
  }

  try
  {
    mVulkanRenderer.Init(mWindow.GetWindow(), mAppName, mEngineName);
    mInputSystem.Init(mWindow.GetWindow());
  }
  catch (const std::exception &e)
  {
    std::cerr << "Vulkan Init Error: " << e.what() << std::endl;
    return false;
  }

  auto camera = mRegistry.create();
  mRegistry.emplace<CameraComponent>(camera);

  auto unit1 = mRegistry.create();
  mRegistry.emplace<TransformComponent>(unit1);

  auto unit2 = mRegistry.create();
  auto &trans = mRegistry.emplace<TransformComponent>(unit2);
  trans.position = {0.0f, 0.0f, 5.0f};

  mIsRunning = true;

  return true;
}

void Engine::Run()
{
  Timer timer;

  while (mIsRunning)
  {
    float dt = timer.Tick();

    ProccessInput(dt);
    Update(dt);
    Render();
  }
}

void Engine::ProccessInput(float dt)
{
  mInputSystem.HandleEvents(mWindow.GetWindow(), mRegistry, dt, mIsRunning);
}

void Engine::Update(float dt)
{
  mMovementSystem.Update(mRegistry, dt);
  mCollisionSystem.Update(mRegistry, dt);
}

void Engine::Render()
{
  if (mWindow.CanRender())
  {
    mVulkanRenderer.DrawFrame(mRegistry);
  }
}