#pragma once
#include <SDL2/SDL.h>
#include <stdexcept>

class Window
{
public:
  Window();
  ~Window();

  void Init(const char *title, int width, int height);
  void Cleanup();
  bool CanRender();
  SDL_Window *GetWindow();

private:
  int mWidth;
  int mHeight;

  SDL_Window *mWindow;
};