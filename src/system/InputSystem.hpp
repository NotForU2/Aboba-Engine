#pragma once
#include "../ecs/Components.hpp"
#include <SDL2/SDL.h>
#include <entt/entt.hpp>

class InputSystem
{
public:
  SDL_Rect GetSelectionRect() const;
  void HandleEvents(entt::registry &registry, bool &isRunning);

private:
  bool mIsSelecting = false;
  SDL_Point mStartPos = {0, 0};
  SDL_Point mCurrentPos = {0, 0};

  SDL_Rect GetNormalizeRect(SDL_Point p1, SDL_Point p2) const;
  void ApplySelection(entt::registry &registry);
};