#include "Window.hpp"
#include "Logger.hpp"

Window::Window() {}

Window::~Window()
{
  SDL_DestroyRenderer(mRenderer);
  SDL_DestroyWindow(mWindow);
}

bool Window::Initialize(const std::string &title, int width, int height)
{
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    return false;
  }

  mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

  if (!mWindow)
  {
    Logger::Log(LogLevel::Error, "Failed to create window!");
    return false;
  }

  mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!mRenderer)
  {
    Logger::Log(LogLevel::Error, "Failed to create renderer!");
    return false;
  }

  mWidth = width;
  mHeight = height;

  return true;
}

void Window::Clear()
{
  SDL_SetRenderDrawColor(mRenderer, 15, 15, 25, 255);
  SDL_RenderClear(mRenderer);
}

void Window::Present()
{
  SDL_RenderPresent(mRenderer);
}