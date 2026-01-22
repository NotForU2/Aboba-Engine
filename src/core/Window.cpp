#include "Window.hpp"
#include "Logger.hpp"

Window::Window(const std::string &title, int width, int height)
{
  mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

  if (!mWindow)
  {
    Logger::Log(LogLevel::Error, "Failed to create window!");
  }

  mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

Window::~Window()
{
  SDL_DestroyRenderer(mRenderer);
  SDL_DestroyWindow(mWindow);
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