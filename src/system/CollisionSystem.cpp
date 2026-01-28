#include "CollisionSystem.hpp"

void CollisionSystem::Update(entt::registry &registry, float dt)
{
  auto view = registry.view<Position, Collider>();

  view.each([&](auto entityA, auto &posA, const auto &colA)
            {
              if (colA.isStatic)
              {
                return;
              }

              view.each([&](auto entityB, const auto &posB, const auto &colB)
            {
              if (entityA == entityB)
              {
                return;
              }

              float dx = posA.x - posB.x;
              float dy = posA.y - posB.y;
              float distSq = dx*dx + dy*dy;

              float minDist = colA.radius + colB.radius;

              if (distSq < minDist * minDist && distSq > 0.0001f)
              {
                float dist = std::sqrt(distSq);
                float overlap = minDist - dist;

                float pushX = dx / dist;
                float pushY = dy / dist;

                float pushFactor = 0.5f;

                if (colB.isStatic)
                {
                  pushFactor = 1.0f;
                }

                float strength = 2.0f;

                posA.x += pushX * overlap * pushFactor * strength * dt;
                posA.y += pushY * overlap * pushFactor * strength * dt;
              }
            }); });
}