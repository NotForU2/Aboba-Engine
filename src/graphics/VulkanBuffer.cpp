#include "VulkanBuffer.hpp"

VulkanBuffer::VulkanBuffer(VulkanBuffer &&other) noexcept
{
  mBuffer = other.mBuffer;
  mAllocation = other.mAllocation;
  mSize = other.mSize;

  other.mBuffer = VK_NULL_HANDLE;
  other.mAllocation = VK_NULL_HANDLE;
  other.mSize = 0;
}

VulkanBuffer &VulkanBuffer::operator=(VulkanBuffer &&other) noexcept
{
  if (this != &other)
  {
    mBuffer = other.mBuffer;
    mAllocation = other.mAllocation;
    mSize = other.mSize;

    other.mBuffer = VK_NULL_HANDLE;
    other.mAllocation = VK_NULL_HANDLE;
    other.mSize = 0;
  }
  return *this;
}

void VulkanBuffer::Create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaFlags)
{
  mSize = size;

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = bufferUsage,
  };
  VmaAllocationCreateInfo allocInfo{
      .flags = vmaFlags,
      .usage = memoryUsage,
  };

  if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &mBuffer, &mAllocation, nullptr) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create generic buffer");
  }
}

void VulkanBuffer::Destroy(VmaAllocator allocator)
{
  if (mBuffer != VK_NULL_HANDLE && mAllocation != VK_NULL_HANDLE)
  {
    vmaDestroyBuffer(allocator, mBuffer, mAllocation);
    mBuffer = VK_NULL_HANDLE;
    mAllocation = VK_NULL_HANDLE;
    mSize = 0;
  }
}

void VulkanBuffer::Upload(VmaAllocator allocator, const void *data, size_t size)
{
  void *mappedData;

  if (vmaMapMemory(allocator, mAllocation, &mappedData) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to map buffer memort");
  }

  memcpy(mappedData, data, size);
  vmaUnmapMemory(allocator, mAllocation);
}

VkDeviceAddress VulkanBuffer::GetDeviceAddress(VkDevice device) const
{
  VkBufferDeviceAddressInfo bufferDeviceAddressInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = mBuffer,
  };

  return vkGetBufferDeviceAddress(device, &bufferDeviceAddressInfo);
}

void *VulkanBuffer::Map(VmaAllocator allocator)
{
  void *mappedData = nullptr;

  if (vmaMapMemory(allocator, mAllocation, &mappedData) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to map buffer memort");
  }

  return mappedData;
}

void VulkanBuffer::Unmap(VmaAllocator allocator)
{
  vmaUnmapMemory(allocator, mAllocation);
}