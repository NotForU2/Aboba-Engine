#include "Engine.hpp"

Engine::Engine() : mIsRunning(false) {}

Engine::~Engine()
{
  mRenderer.WaitIdle();

  mAssetManager.Cleanup();
  mRenderer.Cleanup();
  mContext.Cleanup();
  mWindow.Cleanup();
}

bool Engine::Init()
{
  try
  {
    mWindow.Init(mAppName, 800, 600);
    mContext.Init(&mWindow, mAppName, mEngineName);
    mRenderer.Init(&mContext);
    mAssetManager.Init(&mContext);

    mAssetManager.LoadMesh("panda", "models/Panda.obj");
    mAssetManager.CreateQuad("ground", 1.0f);

    mScene.Init(&mAssetManager);

    mInputSystem.Init(mWindow.GetGLFWwindow());
  }
  catch (const std::exception &e)
  {
    std::cerr << "Init Error: " << e.what() << std::endl;
    return false;
  }

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
  mInputSystem.HandleEvents(mWindow.GetGLFWwindow(), mScene.GetRegistry(), dt, mIsRunning);
}

void Engine::Update(float dt)
{
  mScene.Update(dt);
  mMovementSystem.Update(mScene.GetRegistry(), dt);
  mCollisionSystem.Update(mScene.GetRegistry(), dt);
}

void Engine::Render()
{
  if (mWindow.CanRender())
  {
    auto renderData = mScene.ExtractRenderData();
    float aspect = static_cast<float>(mWindow.GetWindowWidth()) / static_cast<float>(mWindow.GetWindowHeight());
    auto cameraData = mScene.ExtractCameraData(aspect);

    mRenderer.DrawFrame(renderData, cameraData);
  }
}