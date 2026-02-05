#include "Window.hpp"
#include "Logger.hpp"
#include <SDL_vulkan.h>

Window::Window() {}

Window::~Window()
{
  Cleanup();
}

SDL_Window *Window::GetWindow()
{
  return mWindow;
}

void Window::Init(const char *title, int width, int height)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    throw std::runtime_error("SDL_Init failed");
  }

  mWindow = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  if (!mWindow)
  {
    throw std::runtime_error("Failed to create window");
  }

  mWidth = width;
  mHeight = height;
}

bool Window::CanRender()
{
  Uint32 notPresent = SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MINIMIZED;
  return !notPresent;
}

void Window::Cleanup()
{
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}