#pragma once
#include <GLFW/glfw3.h>

class Timer
{
public:
  Timer()
  {
    mFrequency = glfwGetTimerFrequency();
    Reset();
  }

  void Reset()
  {
    mLastTime = glfwGetTimerValue();
  }

  float Tick()
  {
    uint64_t currentTime = glfwGetTimerValue();
    uint64_t deltaTicks = currentTime - mLastTime;
    float dt = static_cast<float>(deltaTicks) / static_cast<float>(mFrequency);

    mLastTime = currentTime;

    if (dt > 0.1f)
    {
      dt = 0.1f;
    }

    return dt;
  }

private:
  uint64_t mFrequency;
  uint64_t mLastTime;
};