#pragma once
#include <entt/entt.hpp>
#include <cmath>
#include "../ecs/Components.hpp"

class MovementSystem
{
public:
  void Update(entt::registry &registry, float dt)
  {
    auto view = registry.view<Position, Destination>();

    view.each([&](auto entity, auto &pos, auto &dest)
              { float dx = dest.targetX - pos.x;
              float dy = dest.targetY - pos.y;
              float distance = std::sqrt(dx * dx + dy * dy);

              if (distance > 1.0f)
              {
                float speed = 150.0f;
                pos.x += (dx / distance) * speed * dt;
                pos.y += (dy / distance) * speed * dt;
              } else {
                registry.remove<Destination>(entity);
              } });
  }
};