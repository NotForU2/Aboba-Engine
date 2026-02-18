#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <array>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <optional>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"
#include "VulkanTexture.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "Vertex.hpp"
#include "../ecs/CameraComponent.hpp"
#include "../ecs/TransformComponent.hpp"

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject
{
  glm::mat4 view;
  glm::mat4 proj;
};

class VulkanRenderer
{
public:
  bool mFramebufferResized = false;

  void Init(GLFWwindow *window, const char *appName, const char *engineName);
  void Cleanup();
  void DrawFrame(entt::registry &registry);
  void DeviceWaitIdle();

private:
  VulkanContext mContext;
  VulkanSwapchain mSwapchain;
  VulkanPipeline mPipeline;
  std::vector<VkCommandBuffer> mCommandBuffers;
  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;
  uint32_t mCurrentFrame = 0;
  VulkanBuffer mVertexBuffer;
  VulkanBuffer mIndexBuffer;
  uint32_t mIndicesCount = 0;
  VkDescriptorSetLayout mDescriptorSetLayout;
  VkDescriptorPool mDescriptorPool;
  std::vector<VkDescriptorSet> mDescriptorSets;
  std::vector<VulkanBuffer> mUniformBuffers;
  std::vector<void *> mUniformBuffersMapped;
  VulkanTexture mTexture;
  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;
  VkImage mDepthImage;
  VmaAllocation mDepthAllocation;
  VkImageView mDepthImageView;
  VkFormat mDepthFormat = VK_FORMAT_D32_SFLOAT;

  void SetFramebufferSizeCallback(GLFWwindow *window);
  void CreatePipelineBarrierEntry(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreatePipelineBarrierOut(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateCommandBuffers();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, entt::registry &registry);
  void CreateSyncObjects();
  void CreateBuffers();
  void CreateDescriptorSetLayout();
  void CreateUniformBuffers();
  void CreateDescriptorPool();
  void CreateDescriptorSets();
  void UpdateUniformBuffer(uint32_t currentImage, entt::registry &registry);
  void LoadModel();
  void CreateDepthResources();
  void RecreateSwapchain();
};