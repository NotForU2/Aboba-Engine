#include "Engine.hpp"

Engine::Engine() : mIsRunning(false) {}

Engine::~Engine()
{
  SDL_Quit();
}

bool Engine::Initialize()
{
  if (!mWindow.Initialize("Aboba Engine", 1280, 720))
  {
    return false;
  }

  auto unit1 = mRegistry.create();
  mRegistry.emplace<Position>(unit1, 100.0f, 100.0f);
  mRegistry.emplace<RenderData>(unit1, 32, 255, 0, 0);

  auto unit2 = mRegistry.create();
  mRegistry.emplace<Position>(unit2, 200.0f, 150.0f);
  mRegistry.emplace<RenderData>(unit2, 32, 0, 0, 255);

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
  mInputSystem.HandleEvents(mRegistry, mIsRunning);
}

void Engine::Update(float dt)
{
  mMovementSystem.Update(mRegistry, dt);
}

void Engine::Render()
{
  mWindow.Clear();
  mRenderSystem.Render(mRegistry, mWindow.GetRenderer(), mInputSystem);
  mWindow.Present();
}