#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>

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

  void CreateInstance();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSurface();
};