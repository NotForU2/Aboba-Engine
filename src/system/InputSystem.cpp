#include "InputSystem.hpp"

Rect InputSystem::GetNormalizeRect(Point p1, Point p2) const
{
  Rect r;

  r.x = std::min(p1.x, p2.x);
  r.y = std::min(p1.y, p2.y);
  r.w = std::abs(p1.x - p2.x);
  r.h = std::abs(p1.y - p2.y);

  return r;
}

void InputSystem::ApplySelection(entt::registry &registry)
{

  Rect selRect = GetNormalizeRect(mStartPos, mCurrentPos);

  bool isClick = (selRect.w < 5 && selRect.h < 5);

  auto view = registry.view<Position, RenderData>();

  // view.each([&](auto entity, const auto &pos, const auto &data)
  //           { Rect unitRect = { (int)pos.x,
  //                               (int)pos.y,
  //                               data.size,
  //                               data.size };

  //             if (isClick)
  //             {
  //               Point clickPoint = {mStartPos.x, mStartPos.y};
  //               if (SDL_PointInRect(&clickPoint, &unitRect))
  //               {
  //                 registry.emplace<Selected>(entity);
  //                 return;
  //               }
  //             }
  //             else
  //             {
  //               if (SDL_HasIntersection(&selRect, &unitRect))
  //               {
  //                 registry.emplace<Selected>(entity);
  //               }
  //             } });
};

Rect InputSystem::GetSelectionRect() const
{
  if (!mIsSelecting)
  {
    return {0, 0, 0, 0};
  }

  return GetNormalizeRect(mStartPos, mCurrentPos);
};

void InputSystem::HandleEvents(GLFWwindow *window, entt::registry &registry, bool &isRunning)
{
  glfwPollEvents();

  if (glfwWindowShouldClose(window))
  {
    isRunning = false;
  }
  // SDL_Event event;

  // while (SDL_PollEvent(&event))
  // {
  //   if (event.type == SDL_QUIT)
  //   {
  //     isRunning = false;
  //   }

  //   if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
  //   {
  //     mIsSelecting = true;
  //     SDL_GetMouseState(&mStartPos.x, &mStartPos.y);
  //     mCurrentPos = mStartPos;

  //     registry.clear<Selected>();
  //   }

  //   if (event.type == SDL_MOUSEMOTION && mIsSelecting)
  //   {
  //     SDL_GetMouseState(&mCurrentPos.x, &mCurrentPos.y);
  //   }

  //   if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
  //   {
  //     mIsSelecting = false;
  //     SDL_GetMouseState(&mCurrentPos.x, &mCurrentPos.y);
  //     ApplySelection(registry);
  //   }

  //   if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
  //   {
  //     int mx, my;
  //     SDL_GetMouseState(&mx, &my);

  //     auto view = registry.view<Position, Selected>();
  //     view.each([&](auto entity, const auto &pos)
  //               { registry.emplace_or_replace<Destination>(entity, (float)mx, (float)my); });
  //   }
  // }
}