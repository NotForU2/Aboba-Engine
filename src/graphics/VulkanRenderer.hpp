#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <optional>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include "wrappers/VulkanBuffer.hpp"
#include "wrappers/VulkanContext.hpp"

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct SwapchainSupportDetails
{
  VkSurfaceCapabilities2KHR capabilities;
  std::vector<VkSurfaceFormat2KHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex
{
  glm::vec2 pos;
  glm::vec3 color;
};

class VulkanRenderer
{
public:
  void Init(SDL_Window *window, const char *appName, const char *engineName);
  void Cleanup();
  void DrawFrame();
  void DeviceWaitIdle();

private:
  VulkanContext mContext;
  // Swapchain
  VkSwapchainKHR mSwapchain;
  std::vector<VkImage> mSwapchainImages;
  VkFormat mSwapchainImageFormat;
  VkExtent2D mSwapchainExtent;
  std::vector<VkImageView> mSwapchainImageViews;
  // Pipeline
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  // Command Buffers
  std::vector<VkCommandBuffer> mCommandBuffers;
  // Sync
  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;
  uint32_t mCurrentFrame = 0;
  // Buffer
  VulkanBuffer mVertexBuffer;
  VulkanBuffer mIndexBuffer;
  uint32_t mIndicesCount = 0;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
  };

  // Swapchain
  void CreateSwapchain();
  void CreateImageViews();
  SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device);
  VkSurfaceFormat2KHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilities2KHR &capabilities);
  // Pipeline
  static std::vector<char> ReadFile(const std::string &filename);
  VkShaderModule CreateShaderModule(const std::vector<char> &code);
  void CreateGraphicsPipeline();
  // Pipeline Barrier
  void CreatePipelineBarrierEntry(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreatePipelineBarrierOut(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  // Command Pool
  void CreateCommandBuffers();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  // Sync
  void CreateSyncObjects();
  // VMA
  void CreateBuffers();
};