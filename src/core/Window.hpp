#pragma once
#include <SDL2/SDL.h>

class Window
{
public:
  Window();
  ~Window();

  bool Init(const char *title, int width, int height);
  void Clear();
  void Present();
  SDL_Window *GetWindow();
  // SDL_Renderer *GetRenderer() const { return mRenderer; }

private:
  int mWidth;
  int mHeight;

  SDL_Window *mWindow;
  // SDL_Renderer *mRenderer;
};