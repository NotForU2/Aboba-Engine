#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstring>
#include <stdexcept>

class VulkanBuffer
{
public:
  VulkanBuffer() = default;
  VulkanBuffer(const VulkanBuffer &) = delete;
  VulkanBuffer &operator=(const VulkanBuffer &) = delete;

  VulkanBuffer(VulkanBuffer &&other) noexcept;
  VulkanBuffer &operator=(VulkanBuffer &&other) noexcept;

  void Create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaFlags = 0);
  void Destroy(VmaAllocator allocator);
  void Upload(VmaAllocator allocator, const void *data, size_t size);
  void *Map(VmaAllocator allocator);
  void Unmap(VmaAllocator allocator);

  VkBuffer GetBuffer() const
  {
    return mBuffer;
  }
  VkDeviceSize GetSize() const { return mSize; }

  VkDeviceAddress GetDeviceAddress(VkDevice device) const;

private:
  VkBuffer mBuffer = VK_NULL_HANDLE;
  VmaAllocation mAllocation = VK_NULL_HANDLE;
  VkDeviceSize mSize = 0;
};