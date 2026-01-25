#pragma once
#include "../ecs/Components.hpp"
#include <SDL2/SDL.h>
#include <entt/entt.hpp>

class InputSystem
{
public:
  void HandleEvents(entt::registry &registry, bool &isRunning)
  {
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        isRunning = false;
      }

      if (event.type == SDL_MOUSEBUTTONDOWN)
      {
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          int mx, my;
          SDL_GetMouseState(&mx, &my);

          auto view = registry.view<entt::entity>();

          view.each([&](auto entity)
                    { registry.emplace_or_replace<Destination>(entity, (float)mx, (float)my); });
        }
      }
    }
  }
};