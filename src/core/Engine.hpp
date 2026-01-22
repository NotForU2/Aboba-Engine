#pragma once
#include "Window.hpp"
#include <memory>

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

  std::unique_ptr<Window> mWindow;
  bool mIsRunning;
};