#pragma once
#include "../ecs/Components.hpp"
#include "../ecs/CameraComponent.hpp"
#include "../geometry/Geometry.hpp"
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

struct InputState
{
  float scrollDelta = 0.0f;
  double lastX = 0.0;
  double lastY = 0.0;
  bool firstMouse = true;
};

class InputSystem
{
public:
  void Init(GLFWwindow *window);
  void HandleEvents(GLFWwindow *window, entt::registry &registry, float dt, bool &isRunning);

private:
  InputState mState;
};