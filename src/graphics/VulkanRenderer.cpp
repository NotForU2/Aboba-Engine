#include "VulkanRenderer.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void VulkanRenderer::Init(GLFWwindow *window, const char *appName, const char *engineName)
{
  mContext.Init(window, appName, engineName);
  SetFramebufferSizeCallback(window);

  mSwapchain.Create(mContext);

  CreateDepthResources();

  CreateDescriptorSetLayout();

  mPipeline.Create(mContext, "shaders/vert.spv", "shaders/frag.spv", mDescriptorSetLayout, mSwapchain.GetImageFormat(), mDepthFormat);

  LoadModel();
  CreateBuffers();
  CreateUniformBuffers();

  mTexture.Create(mContext, "textures/Image_1.jpg");

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

  mPipeline.Destroy(mContext);

  vkDestroyDescriptorSetLayout(mContext.GetDevice(), mDescriptorSetLayout, nullptr);

  vkDestroyImageView(mContext.GetDevice(), mDepthImageView, nullptr);
  vmaDestroyImage(mContext.GetAllocator(), mDepthImage, mDepthAllocation);

  mSwapchain.Destroy(mContext);
  mTexture.Destroy(mContext);
  mContext.Cleanup();
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
}

void VulkanRenderer::CreateSyncObjects()
{
  mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mRenderFinishedSemaphores.resize(mSwapchain.GetImages().size());
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

  for (size_t i = 0; i != mSwapchain.GetImages().size(); ++i)
  {
    if (vkCreateSemaphore(mContext.GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create sync objects for a frame");
    }
  }
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
      .image = mSwapchain.GetImage(imageIndex),
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
      .image = mSwapchain.GetImage(imageIndex),
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

void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, entt::registry &registry)
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
      .imageView = mSwapchain.GetImageView(imageIndex),
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

  VkRenderingAttachmentInfo depthAttachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = mDepthImageView,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue = {
          .depthStencil = {1.0f, 0},
      },
  };

  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {
          .offset = {
              0,
              0,
          },
          .extent = mSwapchain.GetExtent(),
      },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment,
  };

  // Render start
  vkCmdBeginRendering(commandBuffer, &renderingInfo);
  mPipeline.Bind(commandBuffer);

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(mSwapchain.GetExtent().width),
      .height = static_cast<float>(mSwapchain.GetExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{
      .offset = {
          0,
          0,
      },
      .extent = mSwapchain.GetExtent(),
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Bind buffer
  std::vector<VkBuffer> vertexBuffer = {mVertexBuffer.GetBuffer()};
  std::vector<VkDeviceSize> offsets = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.data(), offsets.data());

  vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.GetPipelineLayout(), 0, 1, &mDescriptorSets[mCurrentFrame], 0, nullptr);

  auto view = registry.view<TransformComponent>();

  for (auto entity : view)
  {
    auto &transform = view.get<TransformComponent>(entity);

    // Подготавливаем данные для Push Constant
    MeshPushConstants constants{};
    constants.model = transform.GetModelMatrix();

    // Отправляем данные ПРЯМО в командный буфер
    vkCmdPushConstants(
        commandBuffer,
        mPipeline.GetPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(MeshPushConstants),
        &constants);

    // Рисуем меш (пока используем общий индексный буфер)
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);
  }

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

void VulkanRenderer::DrawFrame(entt::registry &registry)
{
  // Ждем завершения предыдущего кадра
  vkWaitForFences(mContext.GetDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

  // Индекс картинки из Swapchain
  uint32_t imageIndex;
  VkResult acquireNextImageResult = vkAcquireNextImageKHR(mContext.GetDevice(), mSwapchain.GetSwapchain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

  if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
  {
    RecreateSwapchain();
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
  RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex, registry);

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

  UpdateUniformBuffer(mCurrentFrame, registry);

  if (vkQueueSubmit2(mContext.GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit draw command biffer");
  }

  // Present
  std::vector<VkSwapchainKHR> swapchains = {mSwapchain.GetSwapchain()};
  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &mRenderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = swapchains.data(),
      .pImageIndices = &imageIndex,
  };

  VkResult queuePresentResult = vkQueuePresentKHR(mContext.GetPresentQueue(), &presentInfo);

  if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || mFramebufferResized)
  {
    mFramebufferResized = false;
    RecreateSwapchain();
  }
  else if (queuePresentResult != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to present swap chain image");
  }

  mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::CreateBuffers()
{
  mIndicesCount = static_cast<uint32_t>(mIndices.size());

  VkDeviceSize vertexBufferSize = sizeof(Vertex) * mVertices.size();
  VkDeviceSize indexBufferSize = sizeof(uint32_t) * mIndices.size();

  // Vertex
  VulkanBuffer stagingBufferVertex;
  stagingBufferVertex.Create(mContext.GetAllocator(),
                             vertexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_MEMORY_USAGE_AUTO,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  stagingBufferVertex.Upload(mContext.GetAllocator(), mVertices.data(), vertexBufferSize);

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
  stagingBufferIndex.Upload(mContext.GetAllocator(), mIndices.data(), indexBufferSize);

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
  VkDescriptorSetLayoutBinding uboLayoutBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
  };

  VkDescriptorSetLayoutBinding samplerLayoutBinding{
      .binding = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  if (vkCreateDescriptorSetLayout(mContext.GetDevice(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor set layout");
  }
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
}

void VulkanRenderer::CreateDescriptorPool()
{
  VkDescriptorPoolSize matrixPoolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT,
  };

  VkDescriptorPoolSize texPoolSize{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT,
  };

  std::array<VkDescriptorPoolSize, 2> poolSizes{matrixPoolSize, texPoolSize};

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };

  if (vkCreateDescriptorPool(mContext.GetDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool");
  }
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
    VkWriteDescriptorSet bufferDescriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mDescriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };

    VkDescriptorImageInfo imageInfo{
        .sampler = mTexture.GetSampler(),
        .imageView = mTexture.GetImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet imageDescriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mDescriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
    };

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{bufferDescriptorWrite, imageDescriptorWrite};

    vkUpdateDescriptorSets(mContext.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage, entt::registry &registry)
{
  auto view = registry.view<CameraComponent>();
  if (view.empty())
  {
    return;
  }

  const auto &camera = view.get<CameraComponent>(view.front());
  float aspectRatio = static_cast<float>(mSwapchain.GetExtent().width) / static_cast<float>(mSwapchain.GetExtent().height);
  auto proj = glm::perspective(glm::radians(camera.fov), aspectRatio, 0.1f, 100.0f);
  proj[1][1] *= -1;

  UniformBufferObject ubo{
      .view = camera.GetModelMatrix(),
      .proj = proj,
  };

  memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderer::LoadModel()
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  std::string modelPath = "models/Panda.obj";

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
  {
    throw std::runtime_error(warn + err);
  }

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};

  for (const auto &shape : shapes)
  {
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      if (index.texcoord_index >= 0)
      {
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0)
      {
        uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
        mVertices.push_back(vertex);
      }
      mIndices.push_back(uniqueVertices[vertex]);
    }
  }
}

void VulkanRenderer::CreateDepthResources()
{
  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = mDepthFormat,
      .extent = {
          .width = mSwapchain.GetExtent().width,
          .height = mSwapchain.GetExtent().height,
          .depth = 1,
      },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocInfo{
      .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  };

  if (vmaCreateImage(mContext.GetAllocator(), &imageInfo, &allocInfo, &mDepthImage, &mDepthAllocation, nullptr) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create depth image");
  }

  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = mDepthImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };

  if (vkCreateImageView(mContext.GetDevice(), &viewInfo, nullptr, &mDepthImageView) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create depth image view");
  }

  VkCommandBuffer commandBuffer = mContext.BeginSingleTimeCommands();

  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
      .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .image = mDepthImage,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
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

  mContext.EndSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::RecreateSwapchain()
{
  int width = 0;
  int height = 0;

  glfwGetFramebufferSize(mContext.GetWindow(), &width, &height);

  while (width == 0 || height == 0)
  {
    glfwGetFramebufferSize(mContext.GetWindow(), &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(mContext.GetDevice());

  mSwapchain.Recreate(mContext);

  vkDestroyImageView(mContext.GetDevice(), mDepthImageView, nullptr);
  vmaDestroyImage(mContext.GetAllocator(), mDepthImage, mDepthAllocation);

  CreateDepthResources();
}

void VulkanRenderer::SetFramebufferSizeCallback(GLFWwindow *window)
{
  glfwSetWindowUserPointer(window, this);

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
                                 {
    auto self = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    self->mFramebufferResized = true; });
};