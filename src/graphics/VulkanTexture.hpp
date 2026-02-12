#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include "VulkanContext.hpp"
#include "VulkanBuffer.hpp"

class VulkanTexture
{
public:
  void Create(VulkanContext &context, const std::string &filepath);
  void Destroy(VulkanContext &context);
  VkImageView GetImageView() const { return mImageView; }
  VkSampler GetSampler() const { return mSampler; }

private:
  VkImage mImage;
  VmaAllocation mAllocation;
  VkImageView mImageView;
  VkSampler mSampler;

  int mWidth;
  int mHeight;
  int mChannels;

  void CreateImage(VulkanContext &context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
  void TransitionImageLayout(VulkanContext &context, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void CopyBufferToImage(VulkanContext &context, VkBuffer buffer, uint32_t width, uint32_t height);
};