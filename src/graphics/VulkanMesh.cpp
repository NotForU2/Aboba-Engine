#include "VulkanMesh.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void VulkanMesh::UploadBuffers(VulkanContext *context, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  indexCount = static_cast<uint32_t>(indices.size());

  VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
  VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

  // Vertex
  VulkanBuffer stagingBufferVertex;
  stagingBufferVertex.Create(context->GetAllocator(),
                             vertexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_MEMORY_USAGE_AUTO,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBufferVertex.Upload(context->GetAllocator(), vertices.data(), vertexBufferSize);

  vertexBuffer.Create(context->GetAllocator(),
                      vertexBufferSize,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                      0);

  context->CopyBuffer(stagingBufferVertex.GetBuffer(), vertexBuffer.GetBuffer(), vertexBufferSize);

  stagingBufferVertex.Destroy(context->GetAllocator());

  // Index
  VulkanBuffer stagingBufferIndex;
  stagingBufferIndex.Create(context->GetAllocator(),
                            indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VMA_MEMORY_USAGE_AUTO,
                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBufferIndex.Upload(context->GetAllocator(), indices.data(), indexBufferSize);

  indexBuffer.Create(context->GetAllocator(),
                     indexBufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                     0);

  context->CopyBuffer(stagingBufferIndex.GetBuffer(), indexBuffer.GetBuffer(), indexBufferSize);

  stagingBufferIndex.Destroy(context->GetAllocator());
}

void VulkanMesh::CreateQuad(VulkanContext *context, float size)
{
  float halfSize = size / 2.0f;

  std::vector<Vertex> vertices = {
      {{-halfSize, 0.0f, -halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{halfSize, 0.0f, -halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{halfSize, 0.0f, halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-halfSize, 0.0f, halfSize}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
  };

  std::vector<uint32_t> indices = {0, 3, 2, 2, 1, 0};

  UploadBuffers(context, vertices, indices);
}

void VulkanMesh::Destroy(VulkanContext *context)
{
  vertexBuffer.Destroy(context->GetAllocator());
  indexBuffer.Destroy(context->GetAllocator());
}

void VulkanMesh::LoadFromFile(VulkanContext *context, const std::string &filepath)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
  {
    throw std::runtime_error(warn + err);
  }

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (const auto &shape : shapes)
  {
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      if (index.texcoord_index >= 0)
      {
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0)
      {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }

  UploadBuffers(context, vertices, indices);
}