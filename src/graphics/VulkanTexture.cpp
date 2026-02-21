#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "VulkanTexture.hpp"

void VulkanTexture::Create(VulkanContext *context, const std::string &filepath)
{
  stbi_uc *pixels = stbi_load(filepath.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = mWidth * mHeight * 4; // RGBA

  if (!pixels)
  {
    throw std::runtime_error("Failed to load texture image by path: " + filepath);
  }

  VulkanBuffer stagingBuffer;
  stagingBuffer.Create(
      context->GetAllocator(),
      imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_AUTO,
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBuffer.Upload(context->GetAllocator(), pixels, static_cast<size_t>(imageSize));

  stbi_image_free(pixels);

  // Создаем Image на GPU
  CreateImage(context, mWidth, mHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO);

  // Транзакция: Undefined -> TransferDst
  TransitionImageLayout(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Копируем буфер в картинку
  CopyBufferToImage(context, stagingBuffer.GetBuffer(), static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight));

  // Транзакция: TransferDst -> ShaderReadOnly
  TransitionImageLayout(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  stagingBuffer.Destroy(context->GetAllocator());

  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = mImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };

  if (vkCreateImageView(context->GetDevice(), &viewInfo, nullptr, &mImageView) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create texture image view");
  }

  VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };

  if (vkCreateSampler(context->GetDevice(), &samplerInfo, nullptr, &mSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create texture sampler");
  }
}

void VulkanTexture::Destroy(VulkanContext *context)
{
  vkDestroySampler(context->GetDevice(), mSampler, nullptr);
  vkDestroyImageView(context->GetDevice(), mImageView, nullptr);
  vmaDestroyImage(context->GetAllocator(), mImage, mAllocation);
}

void VulkanTexture::CreateImage(VulkanContext *context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
{
  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {
          .width = width,
          .height = height,
          .depth = 1,
      },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocInfo{
      .usage = memoryUsage,
  };

  if (vmaCreateImage(context->GetAllocator(), &imageInfo, &allocInfo, &mImage, &mAllocation, nullptr) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image");
  }
}

void VulkanTexture::TransitionImageLayout(VulkanContext *context, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
  VkCommandBuffer commandBuffer = context->BeginSingleTimeCommands();

  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
      .srcAccessMask = VK_ACCESS_2_NONE,
      .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
      .dstAccessMask = VK_ACCESS_2_NONE,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = mImage,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_NONE;

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
  }
  else
  {
    throw std::invalid_argument("unsupported layout transition!");
  }

  VkDependencyInfo dependencyInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

  context->EndSingleTimeCommands(commandBuffer);
}

void VulkanTexture::CopyBufferToImage(VulkanContext *context, VkBuffer buffer, uint32_t width, uint32_t height)
{
  VkCommandBuffer commandBuffer = context->BeginSingleTimeCommands();

  VkBufferImageCopy2 region{
      .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = 0,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
  };

  VkCopyBufferToImageInfo2 copyInfo{
      .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
      .srcBuffer = buffer,
      .dstImage = mImage,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = 1,
      .pRegions = &region,
  };

  vkCmdCopyBufferToImage2(commandBuffer, &copyInfo);

  context->EndSingleTimeCommands(commandBuffer);
}