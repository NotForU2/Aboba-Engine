#include "Window.hpp"
#include "Logger.hpp"
#include <SDL_vulkan.h>

Window::Window() {}

Window::~Window()
{
  // SDL_DestroyRenderer(mRenderer);
  SDL_DestroyWindow(mWindow);
}

SDL_Window *Window::GetWindow()
{
  return mWindow;
}

bool Window::Init(const char *title, int width, int height)
{
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    return false;
  }

  mWindow = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!mWindow)
  {
    std::cerr << "[SDL Error]: " << SDL_GetError() << std::endl;
    return false;
  }

  // if (!mRenderer)
  // {
  //   Logger::Log(LogLevel::Error, "Failed to create renderer!");
  //   return false;
  // }

  mWidth = width;
  mHeight = height;

  return true;
}

void Window::Clear()
{
  // SDL_SetRenderDrawColor(mRenderer, 15, 15, 25, 255);
  // SDL_RenderClear(mRenderer);
}

void Window::Present()
{
  // SDL_RenderPresent(mRenderer);
}