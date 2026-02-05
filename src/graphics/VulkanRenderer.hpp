#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <optional>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct SwapchainSupportDetails
{
  VkSurfaceCapabilities2KHR capabilities;
  std::vector<VkSurfaceFormat2KHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class VulkanRenderer
{
public:
  void Init(SDL_Window *window);
  void Cleanup();
  void DrawFrame();
  void DeviceWaitIdle();

private:
  // Window
  SDL_Window *mWindow;
  // Instance
  VkInstance mInstance;
  // Device
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  QueueFamilyIndices mQueueFamilyIndices;
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  // Surface
  VkSurfaceKHR mSurface;
  VkQueue mPresentQueue;
  // Swapchain
  VkSwapchainKHR mSwapchain;
  std::vector<VkImage> mSwapchainImages;
  VkFormat mSwapchainImageFormat;
  VkExtent2D mSwapchainExtent;
  std::vector<VkImageView> mSwapchainImageViews;
  // Pipeline
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  // Command Pool
  VkCommandPool mCommandPool;
  std::vector<VkCommandBuffer> mCommandBuffers;
  // Sync
  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;
  uint32_t mCurrentFrame = 0;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  // Instance
  void CreateInstance();
  bool CheckValidationLayerSupport();
  // Device
  void PickPhysicalDevice();
  uint32_t RateDeviceSuitability(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  void CreateLogicalDevice();
  // Surface
  void CreateSurface();
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
  void CreateCommandPool();
  void CreateCommandBuffers();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateSyncObjects();
};