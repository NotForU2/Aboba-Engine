#include "Engine.hpp"
#include <SDL2/SDL.h>

Engine::Engine() : mIsRunning(false) {}

Engine::~Engine()
{
  SDL_Quit();
}

bool Engine::Initialize()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    return false;
  }

  mWindow = std::make_unique<Window>("Aboba Engine", 1280, 720);
  mIsRunning = true;

  return true;
}

void Engine::Run()
{
  uint32_t lastTicks = SDL_GetTicks();

  while (mIsRunning)
  {
    uint32_t currentTicks = SDL_GetTicks();
    float dt = (currentTicks - lastTicks) / 1000.0f;
    lastTicks = currentTicks;

    ProccessInput();
    Update(dt);
    Render();
  }
}

void Engine::ProccessInput()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      mIsRunning = false;
    }
  }
}

void Engine::Update(float dt)
{
}

void Engine::Render()
{
  mWindow->Clear();
  mWindow->Present();
}