#pragma once
#include <entt/entt.hpp>
#include <cmath>
#include "../ecs/Components.hpp"

class CollisionSystem
{
public:
  void Update(entt::registry &registry, float dt);
};