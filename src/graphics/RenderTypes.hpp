#pragma once
#include <glm/glm.hpp>

class VulkanMesh;

struct RenderObject
{
  VulkanMesh *mesh;
  glm::mat4 transform;
};

struct CameraRenderData
{
  glm::mat4 view;
  glm::mat4 projection;
};