#pragma once
#include "VulkanContext.hpp"
#include <vector>
#include <algorithm>

struct SwapchainSupportDetails
{
  VkSurfaceCapabilities2KHR capabilities;
  std::vector<VkSurfaceFormat2KHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain
{
public:
  void Create(VulkanContext &context);
  void Destroy(const VulkanContext &context);
  void Recreate(VulkanContext &context);

  VkSwapchainKHR GetSwapchain() const { return mSwapchain; }
  VkFormat GetImageFormat() const { return mImageFormat; }
  VkExtent2D GetExtent() const { return mExtent; }
  const std::vector<VkImageView> &GetImageViews() const { return mImageViews; }
  VkImageView GetImageView(uint32_t index) const { return mImageViews[index]; }
  const std::vector<VkImage> &GetImages() const { return mImages; }
  VkImage GetImage(uint32_t index) const { return mImages[index]; }

private:
  VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
  std::vector<VkImage> mImages;
  std::vector<VkImageView> mImageViews;
  VkFormat mImageFormat;
  VkExtent2D mExtent;

  void CreateSwapchain(VulkanContext &context);
  void CreateImageViews(VulkanContext &context);
  SwapchainSupportDetails QuerySupport(VulkanContext &context);
  VkSurfaceFormat2KHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats);
  VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseExtent(VulkanContext &context, const VkSurfaceCapabilities2KHR &capabilities);
};