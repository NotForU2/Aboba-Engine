#pragma once
#include <SDL2/SDL.h>

class Timer
{
public:
  Timer()
  {
    mFrequency = static_cast<double>(SDL_GetPerformanceFrequency());
    Reset();
  }

  void Reset()
  {
    mLastTime = SDL_GetPerformanceCounter();
  }

  float Tick()
  {
    uint64_t currentTime = SDL_GetPerformanceCounter();
    uint64_t deltaTicks = currentTime - mLastTime;
    float dt = static_cast<float>(deltaTicks / mFrequency);

    mLastTime = currentTime;

    if (dt > 0.1f)
    {
      dt = 0.1f;
    }

    return dt;
  }

private:
  double mFrequency;
  uint64_t mLastTime;
};