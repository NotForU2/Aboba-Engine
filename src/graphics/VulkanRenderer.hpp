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
#include "VulkanMesh.hpp"
#include "Vertex.hpp"
#include "RenderTypes.hpp"
#include "../ecs/CameraComponent.hpp"
#include "../ecs/TransformComponent.hpp"
#include "../ecs/MeshComponent.hpp"

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

  void Init(VulkanContext *context);
  void Cleanup();
  void DrawFrame(const std::vector<RenderObject> &renderQueue, const CameraRenderData &cameraData);
  void WaitIdle();

private:
  VulkanContext *mContext = nullptr;
  VulkanSwapchain mSwapchain;
  VulkanPipeline mPipeline;
  std::vector<VkCommandBuffer> mCommandBuffers;
  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;
  uint32_t mCurrentFrame = 0;
  VkDescriptorSetLayout mDescriptorSetLayout;
  VkDescriptorPool mDescriptorPool;
  std::vector<VkDescriptorSet> mDescriptorSets;
  std::vector<VulkanBuffer> mUniformBuffers;
  std::vector<void *> mUniformBuffersMapped;
  VulkanTexture mTexture;
  VkImage mDepthImage;
  VmaAllocation mDepthAllocation;
  VkImageView mDepthImageView;
  VkFormat mDepthFormat = VK_FORMAT_D32_SFLOAT;

  void SetFramebufferSizeCallback();
  void CreatePipelineBarrierEntry(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreatePipelineBarrierOut(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateCommandBuffers();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<RenderObject> &renderQueue);
  void CreateSyncObjects();
  void CreateDescriptorSetLayout();
  void CreateUniformBuffers();
  void CreateDescriptorPool();
  void CreateDescriptorSets();
  void UpdateUniformBuffer(uint32_t currentImage, const CameraRenderData &cameraData);
  void CreateDepthResources();
  void RecreateSwapchain();
};