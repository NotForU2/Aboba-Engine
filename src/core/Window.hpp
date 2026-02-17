#pragma once
#include "../graphics/VulkanRenderer.hpp"
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
  GLFWwindow *GetWindow();
  void SetResizeCallback(VulkanRenderer *renderer);

private:
  int mWidth;
  int mHeight;

  GLFWwindow *mWindow = nullptr;
};