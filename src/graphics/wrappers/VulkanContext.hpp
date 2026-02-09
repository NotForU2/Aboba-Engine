#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include <optional>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <map>
#include <set>

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }

  std::set<uint32_t> GetUniqueQueueFamisies()
  {
    std::set<uint32_t> uniqueQueueFamisies = {
        graphicsFamily.value(),
        presentFamily.value(),
    };

    return uniqueQueueFamisies;
  }
};

class VulkanContext
{
public:
  void Init(SDL_Window *window, const char *appName, const char *engineName);
  void Cleanup();

  SDL_Window *GetWindow() const { return mWindow; }
  VkInstance GetInstance() const { return mInstance; }
  VkDevice GetDevice() const { return mDevice; }
  VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
  VkSurfaceKHR GetSurface() const { return mSurface; }
  VmaAllocator GetAllocator() const { return mAllocator; }
  VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
  VkQueue GetPresentQueue() const { return mPresentQueue; }
  VkCommandPool GetCommandPool() const { return mCommandPool; }
  uint32_t GetGraphicsFamily() const { return mQueueFamilyIndices.graphicsFamily.value(); }
  uint32_t GetPresentFamily() const { return mQueueFamilyIndices.presentFamily.value(); }

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
  SDL_Window *mWindow;
  VkInstance mInstance;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice;
  VkSurfaceKHR mSurface;
  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;
  VmaAllocator mAllocator;
  VkCommandPool mCommandPool;
  QueueFamilyIndices mQueueFamilyIndices;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
  };

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  bool CheckValidationLayerSupport();
  void CreateInstance(const char *appName, const char *engineName);
  void PickPhysicalDevice();
  uint32_t RateDeviceSuitability(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  void CreateLogicalDevice();
  void CreateSurface();
  void CreateAllocator();
  void CreateCommandPool();
};