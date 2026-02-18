#include "InputSystem.hpp"

void InputSystem::Init(GLFWwindow *window)
{
  glfwSetWindowUserPointer(window, this);
  glfwSetScrollCallback(window, [](GLFWwindow *w, double x, double y)
                        {
        auto self = static_cast<InputSystem*>(glfwGetWindowUserPointer(w));
        self->mState.scrollDelta += static_cast<float>(y); });
}
void InputSystem::HandleEvents(GLFWwindow *window, entt::registry &registry, float dt, bool &isRunning)
{
  glfwPollEvents();

  if (glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    isRunning = false;
    return;
  }

  auto view = registry.view<CameraComponent>();
  for (auto entity : view)
  {
    auto &camera = view.get<CameraComponent>(entity);

    float moveSpeed = 10.0f * dt;
    float zoomSpeed = 2.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
      camera.focusPoint.z += moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
      camera.focusPoint.z -= moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
      camera.focusPoint.x += moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
      camera.focusPoint.x -= moveSpeed;
    }

    if (mState.scrollDelta != 0.0f)
    {
      camera.distance -= mState.scrollDelta * zoomSpeed;

      if (camera.distance < 2.0f)
      {
        camera.distance = 2.0f;
      }
      if (camera.distance > 50.0f)
      {
        camera.distance = 50.0f;
      }

      mState.scrollDelta = 0.0f;
    }
  }
}