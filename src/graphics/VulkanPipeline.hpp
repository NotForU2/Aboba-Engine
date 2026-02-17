#pragma once
#include "VulkanContext.hpp"
#include "Vertex.hpp"
#include <string>
#include <vector>

class VulkanPipeline
{
public:
  void Create(
      VulkanContext &context,
      const std::string &vertFile,
      const std::string &fragFile,
      VkDescriptorSetLayout descriptorSetLayout,
      VkFormat colorAttachmentFormat,
      VkFormat depthAttachmentFormat);
  void Destroy(const VulkanContext &context);
  void Bind(VkCommandBuffer commandBuffer);

  VkPipeline GetPipeline() const
  {
    return mPipeline;
  }
  VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

private:
  VkPipeline mPipeline = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

  VkShaderModule CreateShaderModule(const VulkanContext &context, const std::vector<char> &code);
  static std::vector<char> ReadFile(const std::string &filename);
};