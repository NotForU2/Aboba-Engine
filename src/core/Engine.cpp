#include "Engine.hpp"

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

  auto entity = mRegistry.create();
  mRegistry.emplace<Position>(entity, 100.0f, 100.0f);
  mRegistry.emplace<RenderData>(entity, 20, 255, 0, 0);

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
  mWindow->Clear();

  SDL_Renderer *renderer = mWindow->GetRenderer();
  auto view = mRegistry.view<Position, RenderData>();

  view.each([renderer](const auto &pos, const auto &data)
            {
    SDL_SetRenderDrawColor(renderer, data.r, data.g, data.b, 255);

    SDL_Rect rect = {
        static_cast<int>(pos.x),
        static_cast<int>(pos.y),
        data.size,
        data.size,
    };
    SDL_RenderFillRect(renderer, &rect); });

  mWindow->Present();
}