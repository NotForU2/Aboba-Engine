#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent
{
  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f};
  glm::vec3 scale{1.0f};

  glm::mat4 GetModelMatrix() const
  {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
    model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
    model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});
    model = glm::scale(model, scale);
    return model;
  }
};