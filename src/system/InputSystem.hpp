#pragma once
#include "../ecs/Components.hpp"
#include "../geometry/Geometry.hpp"
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

class InputSystem
{
public:
  Rect GetSelectionRect() const;
  void HandleEvents(GLFWwindow *window, entt::registry &registry, bool &isRunning);

private:
  bool mIsSelecting = false;
  Point mStartPos = {0, 0};
  Point mCurrentPos = {0, 0};

  Rect GetNormalizeRect(Point p1, Point p2) const;
  void ApplySelection(entt::registry &registry);
};