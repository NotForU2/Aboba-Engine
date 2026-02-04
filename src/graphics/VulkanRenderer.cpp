#include "VulkanRenderer.hpp"

void VulkanRenderer::Init(SDL_Window *window)
{
  mWindow = window;

  CreateInstance();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapchain();
  CreateImageViews();
  CreateGraphicsPipeline();
}

void VulkanRenderer::Cleanup()
{
  vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
  for (auto imageView : mSwapchainImageViews)
  {
    vkDestroyImageView(mDevice, imageView, nullptr);
  }
  vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
  vkDestroyDevice(mDevice, nullptr);
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

bool VulkanRenderer::CheckValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers)
  {
    bool layerFound = false;

    for (const VkLayerProperties &layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }

    if (!layerFound)
    {
      return false;
    }
  }

  return true;
}

void VulkanRenderer::CreateInstance()
{
  if (enableValidationLayers && !CheckValidationLayerSupport())
  {
    throw std::runtime_error("Validation layers requested, but not available");
  }

  uint32_t version = 0;

  if (vkEnumerateInstanceVersion(&version) != VK_SUCCESS)
  {
    version = VK_API_VERSION_1_0;
  }

  std::cout << "System supports Vulkan Variant: " << VK_API_VERSION_VARIANT(version)
            << ", Major: " << VK_API_VERSION_MAJOR(version)
            << ", Minor: " << VK_API_VERSION_MINOR(version)
            << ", Patch: " << VK_API_VERSION_PATCH(version) << std::endl;

  uint32_t targetVersion = version;

  if (version >= VK_API_VERSION_1_4)
  {
    targetVersion = VK_API_VERSION_1_4;
  }

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Aboba Engine",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Aboba Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = targetVersion,
  };

  std::cout << "Initializing Vulkan Instance with API Version: 1." << VK_API_VERSION_MINOR(targetVersion) << std::endl;

  uint32_t sdlExtensionCount = 0;
  if (!SDL_Vulkan_GetInstanceExtensions(mWindow, &sdlExtensionCount, nullptr))
  {
    throw std::runtime_error("Failed to get SDL Vulkan extensions count");
  }

  std::vector<const char *> extensions(sdlExtensionCount);
  if (!SDL_Vulkan_GetInstanceExtensions(mWindow, &sdlExtensionCount, extensions.data()))
  {
    throw std::runtime_error("Failed to get SDL Vulkan extensions");
  }

  // 2KHR
  extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

  // В Vulkan 1.3+ для переносимости (особенно на macOS через MoltenVK) может понадобиться флаг (flag)
  // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR, но на Windows пока опустим.
  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
      .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Vulkan Instance");
  }

  std::cout << "Vulkan Instance created successfully" << std::endl;
}

void VulkanRenderer::PickPhysicalDevice()
{
  uint32_t deviceCount = 0;

  if (vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to get GPUs count with Vulkan support");
  }

  if (deviceCount == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  if (vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to get GPUs info with Vulkan support");
  }

  std::multimap<int, VkPhysicalDevice> rating;

  for (const auto &device : devices)
  {
    int score = RateDeviceSuitability(device);
    rating.insert(std::make_pair(score, device));
  }

  if (rating.rbegin()->first > 0)
  {
    mPhysicalDevice = rating.rbegin()->second;
    mQueueFamilyIndices = FindQueueFamilies(mPhysicalDevice);
  }
  else
  {
    throw std::runtime_error("Failed to find a suitable GPU");
  }
}

int VulkanRenderer::RateDeviceSuitability(VkPhysicalDevice device)
{
  VkPhysicalDeviceProperties2 deviceProperties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
  };
  vkGetPhysicalDeviceProperties2(device, &deviceProperties);

  int score = 0;

  if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  {
    score += 1000;
  }

  score += deviceProperties.properties.limits.maxImageDimension2D;
  score += deviceProperties.properties.limits.maxImageDimension3D;

  QueueFamilyIndices indices = FindQueueFamilies(device);
  if (!indices.isComplete())
  {
    return 0;
  }

  // Vulkan 1.2
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
  };
  // Vulkan 1.3
  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &features12,
  };
  // Vulkan 1.4
  VkPhysicalDeviceVulkan14Features features14{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
      .pNext = &features13,
  };
  // Vulkan 1.0
  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &features14,
  };
  vkGetPhysicalDeviceFeatures2(device, &features2);

  if (!features13.dynamicRendering)
  {
    std::cout << "Device (" << deviceProperties.properties.deviceName << ") missing Dynamic Rendering support" << std::endl;
    return 0;
  }
  if (!features13.synchronization2)
  {
    std::cout << "Device (" << deviceProperties.properties.deviceName << ") missing Synchronization2 support" << std::endl;
    return 0;
  }
  if (!features2.features.geometryShader)
  {
    std::cout << "Device (" << deviceProperties.properties.deviceName << ") missing Geometry Shader support" << std::endl;
    return 0;
  }

  std::cout << "Found GPU: " << deviceProperties.properties.deviceName << " (Score: " << score << ")" << std::endl;

  return score;
}

QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);

  VkQueueFamilyProperties2 defaultFamilyProperties2{
      .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
      .pNext = nullptr,
  };
  std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount, defaultFamilyProperties2);
  vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());

  for (size_t i = 0; i != queueFamilies.size(); i++)
  {
    if (queueFamilies[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphicsFamily = static_cast<uint32_t>(i);
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), mSurface, &presentSupport);

    if (presentSupport)
    {
      indices.presentFamily = static_cast<uint32_t>(i);
    }

    if (indices.isComplete())
    {
      break;
    }
  }

  return indices;
}

void VulkanRenderer::CreateLogicalDevice()
{
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamisies = {
      mQueueFamilyIndices.graphicsFamily.value(),
      mQueueFamilyIndices.presentFamily.value(),
  };

  for (uint32_t queieFamily : uniqueQueueFamisies)
  {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queieFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // Vulkan 1.2
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
  };
  // Vulkan 1.3
  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &features12,
      .synchronization2 = VK_TRUE,
      .dynamicRendering = VK_TRUE,
  };
  // Vulkan 1.4
  VkPhysicalDeviceVulkan14Features features14{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
      .pNext = &features13,
  };
  // Vulkan 1.0
  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &features14,
      .features = {
          .geometryShader = VK_TRUE,
          .fillModeNonSolid = VK_TRUE,
          .samplerAnisotropy = VK_TRUE,
      },
  };

  // enabledLayerCount и ppEnabledLayerNames устарели
  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &features2,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
      .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = nullptr,
  };

  if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create logical device");
  }

  VkDeviceQueueInfo2 queueInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
      .queueFamilyIndex = mQueueFamilyIndices.graphicsFamily.value(),
      .queueIndex = 0,
  };

  vkGetDeviceQueue2(mDevice, &queueInfo, &mGraphicsQueue);
  if (mQueueFamilyIndices.graphicsFamily != mQueueFamilyIndices.presentFamily)
  {
    queueInfo.queueFamilyIndex = mQueueFamilyIndices.presentFamily.value();
    vkGetDeviceQueue2(mDevice, &queueInfo, &mPresentQueue);
  }
  else
  {
    mPresentQueue = mGraphicsQueue;
  }

  std::cout << "Logical Device created with Vulkan 1.4 chain" << std::endl;
}

void VulkanRenderer::CreateSurface()
{
  if (!SDL_Vulkan_CreateSurface(mWindow, mInstance, &mSurface))
  {
    throw std::runtime_error("Failed to create window surface");
  }

  std::cout << "Vulkan Surface created successfully" << std::endl;
}

SwapchainSupportDetails VulkanRenderer::QuerySwapchainSupport(VkPhysicalDevice device)
{
  SwapchainSupportDetails details;

  VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
      .surface = mSurface,
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
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
  }

  return details;
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
    SDL_Vulkan_GetDrawableSize(mWindow, &width, &height);

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
  SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(mPhysicalDevice);

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
      .surface = mSurface,
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

  uint32_t queueFamilyIndices[] = {
      mQueueFamilyIndices.graphicsFamily.value(),
      mQueueFamilyIndices.presentFamily.value(),
  };

  if (mQueueFamilyIndices.graphicsFamily != mQueueFamilyIndices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Swapchain");
  }

  vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
  mSwapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

  mSwapchainImageFormat = surfaceFormat.surfaceFormat.format;
  mSwapchainExtent = extent;

  std::cout << "Swapchain created. Images: " << imageCount << ", Resolution: "
            << extent.width << "x" << extent.height << std::endl;
}

void VulkanRenderer::CreateImageViews()
{
  mSwapchainImageViews.resize(mSwapchainImages.size());

  for (size_t i = 0; i != mSwapchainImages.size(); i++)
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

    if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
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
  if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
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
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
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
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
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
  };
  if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
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

  if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create graphics pipeline");
  }

  std::cout << "Vulkan Graphics Pipeline created successfully" << std::endl;

  vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
  vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
}