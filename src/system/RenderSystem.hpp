#pragma once
#include <entt/entt.hpp>
#include <SDL2/SDL.h>
#include "../ecs/Components.hpp"
#include "InputSystem.hpp"

class RenderSystem
{
public:
  void Render(entt::registry &registry, SDL_Renderer *renderer, const InputSystem &inputSystem);
};