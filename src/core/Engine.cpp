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
    mWindow.SetResizeCallback(&mVulkanRenderer);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Vulkan Init Error: " << e.what() << std::endl;
    return false;
  }

  auto unit1 = mRegistry.create();
  mRegistry.emplace<Position>(unit1, 100.0f, 100.0f);
  mRegistry.emplace<RenderData>(unit1, 32, 255, 0, 0);
  mRegistry.emplace<Collider>(unit1, 16.0f, false);

  auto unit2 = mRegistry.create();
  mRegistry.emplace<Position>(unit2, 200.0f, 150.0f);
  mRegistry.emplace<RenderData>(unit2, 32, 0, 0, 255);
  mRegistry.emplace<Collider>(unit2, 16.0f, false);

  mIsRunning = true;

  return true;
}

void Engine::Run()
{
  Timer timer;

  while (mIsRunning)
  {
    float dt = timer.Tick();

    ProccessInput();
    Update(dt);
    Render();
  }
}

void Engine::ProccessInput()
{
  mInputSystem.HandleEvents(mWindow.GetWindow(), mRegistry, mIsRunning);
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
    mVulkanRenderer.DrawFrame();
  }
}