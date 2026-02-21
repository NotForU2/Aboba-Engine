#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>

class Window
{
public:
  Window();
  ~Window();

  void Init(const char *title, int width, int height);
  void Cleanup();
  bool CanRender();
  GLFWwindow *GetGLFWwindow();
  int GetWindowWidth() const { return mWindowWidth; };
  int GetWindowHeight() const { return mWindowHeight; };
  int GetFramebufferWidth() const { return mFramebufferWidth; };
  int GetFramebufferHeight() const { return mFramebufferHeight; };
  void UpdateDimensions();

private:
  int mWindowWidth;
  int mWindowHeight;
  int mFramebufferWidth;
  int mFramebufferHeight;

  GLFWwindow *mWindow = nullptr;
};