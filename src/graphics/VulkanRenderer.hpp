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

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value();
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
  SDL_Window *mWindow;
  VkInstance mInstance;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  VkSurfaceKHR mSurface;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  void CreateInstance();
  bool CheckValidationLayerSupport();
  void PickPhysicalDevice();
  int RateDeviceSuitability(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  void CreateLogicalDevice();
  void CreateSurface();
};