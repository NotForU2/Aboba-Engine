#include "VulkanRenderer.hpp"

void VulkanRenderer::Init(SDL_Window *window, const char *appName, const char *engineName)
{
  mContext.Init(window, appName, engineName);
  mTexture.Create(mContext, "textures/aaa.png");

  CreateSwapchain();
  CreateImageViews();
  mCamera.SetProjection(45.0f, mSwapchainExtent.width / mSwapchainExtent.height, 0.1f, 10.0f);

  CreateDescriptorSetLayout();

  CreateGraphicsPipeline();

  CreateBuffers();
  CreateUniformBuffers();

  CreateDescriptorPool();
  CreateDescriptorSets();

  CreateCommandBuffers();
  CreateSyncObjects();
}

void VulkanRenderer::Cleanup()
{
  vkDeviceWaitIdle(mContext.GetDevice());

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i)
  {
    vkDestroySemaphore(mContext.GetDevice(), mImageAvailableSemaphores[i], nullptr);
    vkDestroyFence(mContext.GetDevice(), mInFlightFences[i], nullptr);
  }
  for (size_t i = 0; i != mRenderFinishedSemaphores.size(); ++i)
  {
    vkDestroySemaphore(mContext.GetDevice(), mRenderFinishedSemaphores[i], nullptr);
  }

  vkDestroyDescriptorPool(mContext.GetDevice(), mDescriptorPool, nullptr);

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i)
  {
    mUniformBuffers[i].Unmap(mContext.GetAllocator());
    mUniformBuffers[i].Destroy(mContext.GetAllocator());
  }
  mVertexBuffer.Destroy(mContext.GetAllocator());
  mIndexBuffer.Destroy(mContext.GetAllocator());

  vkDestroyPipeline(mContext.GetDevice(), mGraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(mContext.GetDevice(), mPipelineLayout, nullptr);

  vkDestroyDescriptorSetLayout(mContext.GetDevice(), mDescriptorSetLayout, nullptr);

  for (auto imageView : mSwapchainImageViews)
  {
    vkDestroyImageView(mContext.GetDevice(), imageView, nullptr);
  }
  vkDestroySwapchainKHR(mContext.GetDevice(), mSwapchain, nullptr);

  mTexture.Destroy(mContext);
  mContext.Cleanup();
}

VkSurfaceFormat2KHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats2)
{
  for (const auto &availableFormat2 : availableFormats2)
  {
    if (availableFormat2.surfaceFormat.format == VK_FORMAT_B8G8R8_SRGB && availableFormat2.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return availableFormat2;
    }
  }

  return availableFormats2[0];
}

VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
  for (const auto &availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilities2KHR &capabilities2)
{
  if (capabilities2.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities2.surfaceCapabilities.currentExtent;
  }
  else
  {
    int width, height;
    SDL_Vulkan_GetDrawableSize(mContext.GetWindow(), &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };

    actualExtent.width = std::clamp(
        actualExtent.width,
        capabilities2.surfaceCapabilities.minImageExtent.width,
        capabilities2.surfaceCapabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(
        actualExtent.height,
        capabilities2.surfaceCapabilities.minImageExtent.height,
        capabilities2.surfaceCapabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void VulkanRenderer::CreateSwapchain()
{
  SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(mContext.GetPhysicalDevice());

  VkSurfaceFormat2KHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

  uint32_t imageCount = swapchainSupport.capabilities.surfaceCapabilities.minImageCount + 1;

  if (swapchainSupport.capabilities.surfaceCapabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.surfaceCapabilities.maxImageCount)
  {
    imageCount = swapchainSupport.capabilities.surfaceCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = mContext.GetSurface(),
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.surfaceFormat.format,
      .imageColorSpace = surfaceFormat.surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = swapchainSupport.capabilities.surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  std::vector<uint32_t> queueFamilyIndices = {
      mContext.GetGraphicsFamily(),
      mContext.GetPresentFamily(),
  };

  if (queueFamilyIndices[0] != queueFamilyIndices[1])
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(mContext.GetDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Swapchain");
  }

  vkGetSwapchainImagesKHR(mContext.GetDevice(), mSwapchain, &imageCount, nullptr);
  mSwapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mContext.GetDevice(), mSwapchain, &imageCount, mSwapchainImages.data());

  mSwapchainImageFormat = surfaceFormat.surfaceFormat.format;
  mSwapchainExtent = extent;

  std::cout << "Swapchain created. Images: " << imageCount << ", Resolution: "
            << extent.width << "x" << extent.height << std::endl;
}

void VulkanRenderer::CreateImageViews()
{
  mSwapchainImageViews.resize(mSwapchainImages.size());

  for (size_t i = 0; i != mSwapchainImages.size(); ++i)
  {
    VkImageViewCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = mSwapchainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = mSwapchainImageFormat,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if (vkCreateImageView(mContext.GetDevice(), &createInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Image Views");
    }
  }
}

std::vector<char> VulkanRenderer::ReadFile(const std::string &filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char> &code)
{
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(mContext.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create shader module");
  }

  return shaderModule;
}

void VulkanRenderer::CreateGraphicsPipeline()
{
  // Загрузка шейдеров
  auto vertShaderCode = ReadFile("shaders/vert.spv");
  auto fragShaderCode = ReadFile("shaders/frag.spv");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertShaderModule,
      .pName = "main",
  };
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragShaderModule,
      .pName = "main",
  };

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
      vertShaderStageInfo,
      fragShaderStageInfo,
  };

  // Vertex Input
  VkVertexInputAttributeDescription attributeDescriptionPosition{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(Vertex, pos),
  };
  VkVertexInputAttributeDescription attributeDescriptionColor{
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Vertex, color),
  };

  std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
      attributeDescriptionPosition,
      attributeDescriptionColor,
  };

  VkVertexInputBindingDescription bindingDescription{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };

  // Input Assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  // Viewport и Scissor
  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  // Color Blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
  };

  // Dynamic States
  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo pipelineDynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  // Pipeline Layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &mDescriptorSetLayout,
  };

  if (vkCreatePipelineLayout(mContext.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  // Pipeline Rendering
  VkPipelineRenderingCreateInfo pipelineRenderingInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &mSwapchainImageFormat,
  };

  // Main create info
  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &pipelineRenderingInfo,
      .stageCount = 2,
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &pipelineDynamicState,
      .layout = mPipelineLayout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
  };

  if (vkCreateGraphicsPipelines(mContext.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create graphics pipeline");
  }

  std::cout << "Vulkan Graphics Pipeline created successfully" << std::endl;

  vkDestroyShaderModule(mContext.GetDevice(), fragShaderModule, nullptr);
  vkDestroyShaderModule(mContext.GetDevice(), vertShaderModule, nullptr);
}

SwapchainSupportDetails VulkanRenderer::QuerySwapchainSupport(VkPhysicalDevice device)
{
  SwapchainSupportDetails details;

  VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
      .surface = mContext.GetSurface(),
  };

  VkSurfaceCapabilities2KHR capabilities2{
      .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
  };
  vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo2, &capabilities2);
  details.capabilities = capabilities2;

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo2, &formatCount, nullptr);
  if (formatCount != 0)
  {
    VkSurfaceFormat2KHR defaultFormat{
        .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
    };
    std::vector<VkSurfaceFormat2KHR> formats2(formatCount, defaultFormat);

    vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo2, &formatCount, formats2.data());

    details.formats = formats2;
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, mContext.GetSurface(), &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mContext.GetSurface(), &presentModeCount, details.presentModes.data());
  }

  return details;
}

void VulkanRenderer::CreateCommandBuffers()
{
  mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = mContext.GetCommandPool(),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size()),
  };
  if (vkAllocateCommandBuffers(mContext.GetDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate command buffers");
  }

  std::cout << "Vulkan Command Buffers created successfully" << std::endl;
}

void VulkanRenderer::CreateSyncObjects()
{
  mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mRenderFinishedSemaphores.resize(mSwapchainImages.size());
  mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i)
  {
    if (vkCreateSemaphore(mContext.GetDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(mContext.GetDevice(), &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create sync objects for a frame");
    }
  }

  for (size_t i = 0; i != mSwapchainImages.size(); ++i)
  {
    if (vkCreateSemaphore(mContext.GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create sync objects for a frame");
    }
  }

  std::cout << "Vulkan Sync Objects created successfully" << std::endl;
}

void VulkanRenderer::CreatePipelineBarrierEntry(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .image = mSwapchainImages[imageIndex],
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };

  VkDependencyInfo dependencyInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
};

void VulkanRenderer::CreatePipelineBarrierOut(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
      .dstAccessMask = 0,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .image = mSwapchainImages[imageIndex],
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };

  VkDependencyInfo dependencyInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
};

void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to begin recording command buffer");
  }

  CreatePipelineBarrierEntry(commandBuffer, imageIndex);

  // Переводим изображение из формата "Непонятно что" в "Куда можно рисовать цвет"
  // Для простоты пока опустим явные барьеры ImageMemoryBarrier,
  // так как Subpass Dependencies в RenderPass делали это за нас,
  // но в Dynamic Rendering Swapchain обычно сам справляется при правильной настройке,
  // однако правильнее будет сделать Transition Layout.
  // ДЛЯ ПЕРВОГО ТРЕУГОЛЬНИКА Vulkan часто прощает отсутствие явного барьера здесь,
  // потом понадобится PipelineBarrier для перехода layout.

  // Настройка Dynamic Rendering
  VkRenderingAttachmentInfo colorAttachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = mSwapchainImageViews[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {
          {{
              0.0f,
              0.0f,
              0.0f,
              0.1f,
          }}},
  };
  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {
          .offset = {
              0,
              0,
          },
          .extent = mSwapchainExtent,
      },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };

  // Render start
  vkCmdBeginRendering(commandBuffer, &renderingInfo);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(mSwapchainExtent.width),
      .height = static_cast<float>(mSwapchainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{
      .offset = {
          0,
          0,
      },
      .extent = mSwapchainExtent,
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Bind buffer
  std::vector<VkBuffer> vertexBuffer = {mVertexBuffer.GetBuffer()};
  std::vector<VkDeviceSize> offsets = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.data(), offsets.data());

  vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[mCurrentFrame], 0, nullptr);
  // Draw
  vkCmdDrawIndexed(commandBuffer, mIndicesCount, 1, 0, 0, 0);
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  vkCmdEndRendering(commandBuffer);

  // Pipeline Barrier
  CreatePipelineBarrierOut(commandBuffer, imageIndex);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to record command buffer");
  }
}

void VulkanRenderer::DrawFrame()
{
  // Ждем завершения предыдущего кадра
  vkWaitForFences(mContext.GetDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

  // Индекс картинки из Swapchain
  uint32_t imageIndex;
  VkResult acquireNextImageResult = vkAcquireNextImageKHR(mContext.GetDevice(), mSwapchain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

  if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
  {
    return;
  }
  else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("Failed to acquire swapchain image");
  }

  // Сброс fances
  vkResetFences(mContext.GetDevice(), 1, &mInFlightFences[mCurrentFrame]);

  // Записываем команды
  vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
  RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

  // Инфо Semaphore ожидания
  VkSemaphoreSubmitInfo waitSemaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = mImageAvailableSemaphores[mCurrentFrame],
      .value = 1,
      .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .deviceIndex = 0,
  };

  // Инфо Semaphore сигнала
  VkSemaphoreSubmitInfo signalSemaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = mRenderFinishedSemaphores[imageIndex],
      .value = 1,
      .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
      .deviceIndex = 0,
  };

  // Инфо Commad Buffer
  VkCommandBufferSubmitInfo commandBufferInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .commandBuffer = mCommandBuffers[mCurrentFrame],
      .deviceMask = 0,
  };

  // Submit Info
  VkSubmitInfo2 submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount = 1,
      .pWaitSemaphoreInfos = &waitSemaphoreInfo,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &commandBufferInfo,
      .signalSemaphoreInfoCount = 1,
      .pSignalSemaphoreInfos = &signalSemaphoreInfo,
  };

  UpdateUniformBuffer(mCurrentFrame);

  if (vkQueueSubmit2(mContext.GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit draw command biffer");
  }

  // Present
  std::vector<VkSwapchainKHR> swapchains = {mSwapchain};
  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &mRenderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = swapchains.data(),
      .pImageIndices = &imageIndex,
  };

  VkResult queuePresentResult = vkQueuePresentKHR(mContext.GetPresentQueue(), &presentInfo);

  if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR)
  {
  }
  else if (queuePresentResult != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to present swap chain image");
  }

  mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::CreateBuffers()
{
  std::vector<Vertex> vertices = {
      {
          {-0.5f, -0.5f},
          {1.0f, 0.0f, 0.0f},
      },
      {
          {0.5f, -0.5f},
          {0.0f, 1.0f, 0.0f},
      },
      {
          {0.5f, 0.5f},
          {0.0f, 0.0f, 1.0f},
      },
      {
          {-0.5f, 0.5f},
          {1.0f, 1.0f, 1.0f},
      },
  };

  std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
  mIndicesCount = static_cast<uint32_t>(indices.size());

  VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
  VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

  // Vertex
  VulkanBuffer stagingBufferVertex;
  stagingBufferVertex.Create(mContext.GetAllocator(),
                             vertexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_MEMORY_USAGE_AUTO,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBufferVertex.Upload(mContext.GetAllocator(), vertices.data(), vertexBufferSize);

  mVertexBuffer.Create(mContext.GetAllocator(),
                       vertexBufferSize,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                       0);

  mContext.CopyBuffer(stagingBufferVertex.GetBuffer(), mVertexBuffer.GetBuffer(), vertexBufferSize);

  stagingBufferVertex.Destroy(mContext.GetAllocator());

  // Index
  VulkanBuffer stagingBufferIndex;
  stagingBufferIndex.Create(mContext.GetAllocator(),
                            indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VMA_MEMORY_USAGE_AUTO,
                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBufferIndex.Upload(mContext.GetAllocator(), indices.data(), indexBufferSize);

  mIndexBuffer.Create(mContext.GetAllocator(),
                      indexBufferSize,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                      0);

  mContext.CopyBuffer(stagingBufferIndex.GetBuffer(), mIndexBuffer.GetBuffer(), indexBufferSize);

  stagingBufferIndex.Destroy(mContext.GetAllocator());
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding uboLayotBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  };
  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &uboLayotBinding,
  };

  if (vkCreateDescriptorSetLayout(mContext.GetDevice(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor set layout");
  }

  std::cout << "Vulkan Descriptor Set Layout created successfully" << std::endl;
}

void VulkanRenderer::CreateUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  mUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i)
  {
    // HOST_VISIBLE | HOST_COHERENT
    // Идеально было бы DEVICE_LOCAL | HOST_VISIBLE (ReBar)
    mUniformBuffers[i].Create(
        mContext.GetAllocator(),
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    mUniformBuffersMapped[i] = mUniformBuffers[i].Map(mContext.GetAllocator());
  }

  std::cout << "Vulkan Uniform Buffers created successfully" << std::endl;
}

void VulkanRenderer::CreateDescriptorPool()
{
  VkDescriptorPoolSize poolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT,
  };
  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
  };

  if (vkCreateDescriptorPool(mContext.GetDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool");
  }

  std::cout << "Vulkan Descriptor Pool created successfully" << std::endl;
}

void VulkanRenderer::CreateDescriptorSets()
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = mDescriptorPool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts = layouts.data(),
  };

  mDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(mContext.GetDevice(), &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate descriptor sets");
  }

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i)
  {
    VkDescriptorBufferInfo bufferInfo{
        .buffer = mUniformBuffers[i].GetBuffer(),
        .offset = 0,
        .range = sizeof(UniformBufferObject),
    };
    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mDescriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };

    vkUpdateDescriptorSets(mContext.GetDevice(), 1, &descriptorWrite, 0, nullptr);
  }

  std::cout << "Vulkan Descriptor Sets created successfully" << std::endl;
}

void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo{
      .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      .view = mCamera.GetView(),
      .proj = mCamera.GetProjection(),
  };

  memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}