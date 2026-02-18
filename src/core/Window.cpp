#include "Window.hpp"
#include "Logger.hpp"
#include "../system/InputSystem.hpp"

Window::Window() {}

Window::~Window()
{
  Cleanup();
}

GLFWwindow *Window::GetWindow()
{
  return mWindow;
}

void Window::Init(const char *title, int width, int height)
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!mWindow)
  {
    throw std::runtime_error("Failed to create window");
  }

  glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  mWidth = width;
  mHeight = height;
}

bool Window::CanRender()
{
  return true;
}

void Window::Cleanup()
{
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}