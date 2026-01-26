#pragma once
#include <SDL2/SDL.h>
#include <string>

class Window
{
public:
  Window();
  ~Window();

  bool Initialize(const std::string &title, int width, int height);
  void Clear();
  void Present();
  SDL_Renderer *GetRenderer() const { return mRenderer; }

private:
  SDL_Window *mWindow;
  SDL_Renderer *mRenderer;
};