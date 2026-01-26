#include "RenderSystem.hpp"

void RenderSystem::Render(entt::registry &registry, SDL_Renderer *renderer, const InputSystem &inputSystem)
{
  auto view = registry.view<Position, RenderData>();
  view.each([&](auto entity, const auto &pos, const auto &data)
            {
      SDL_SetRenderDrawColor(renderer, data.r, data.g, data.b, 255);
      SDL_Rect rect = {
        static_cast<int>(pos.x),
        static_cast<int>(pos.y),
        data.size,
        data.size
      };
      SDL_RenderFillRect(renderer, &rect); 
    
      if (registry.all_of<Selected>(entity))
      {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect outline = {
          rect.x - 2,
          rect.y - 2,
          rect.w + 4,
          rect.h + 4
        };
        SDL_RenderDrawRect(renderer, &outline);
      } });

  SDL_Rect dragRect = inputSystem.GetSelectionRect();
  if (dragRect.w > 0 || dragRect.h > 0)
  {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(renderer, &dragRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 50);
    SDL_RenderFillRect(renderer, &dragRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  }
}