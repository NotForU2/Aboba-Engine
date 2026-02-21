#pragma once
#include "VulkanContext.hpp"
#include "VulkanBuffer.hpp"
#include "Vertex.hpp"
#include <vector>
#include <string>

class VulkanMesh
{
public:
  VulkanBuffer vertexBuffer;
  VulkanBuffer indexBuffer;
  uint32_t indexCount = 0;

  void LoadFromFile(VulkanContext *context, const std::string &filepath);
  void CreateQuad(VulkanContext *context, float size = 1.0f);
  void Destroy(VulkanContext *context);

private:
  void UploadBuffers(VulkanContext *context, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
};