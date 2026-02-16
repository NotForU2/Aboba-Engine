#define VMA_IMPLEMENTATION
#include "VulkanContext.hpp"

void VulkanContext::Init(GLFWwindow *window, const char *appName, const char *engineName)
{
  mWindow = window;

  CreateInstance(appName, engineName);
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateAllocator();
  CreateCommandPool();
}

void VulkanContext::Cleanup()
{
  vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
  vmaDestroyAllocator(mAllocator);
  vkDestroyDevice(mDevice, nullptr);
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

bool VulkanContext::CheckValidationLayerSupport()
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

std::vector<const char *> VulkanContext::GetSdlExtensions()
{
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  // 2KHR
  extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

  return extensions;
}

void VulkanContext::CreateInstance(const char *appName, const char *engineName)
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
      .pApplicationName = appName,
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = engineName,
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = targetVersion,
  };

  std::cout << "Initializing Vulkan Instance with API Version: 1." << VK_API_VERSION_MINOR(targetVersion) << std::endl;

  auto extensions = GetSdlExtensions();

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

void VulkanContext::PickPhysicalDevice()
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

uint32_t VulkanContext::RateDeviceSuitability(VkPhysicalDevice device)
{
  VkPhysicalDeviceProperties2 deviceProperties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
  };
  vkGetPhysicalDeviceProperties2(device, &deviceProperties);

  uint32_t score = 0;

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

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
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

  for (size_t i = 0; i != queueFamilies.size(); ++i)
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

void VulkanContext::CreateLogicalDevice()
{
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamisies = mQueueFamilyIndices.GetUniqueQueueFamilies();

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
      .bufferDeviceAddress = VK_TRUE,

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

void VulkanContext::CreateSurface()
{
  if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface");
  }

  std::cout << "Vulkan Surface created successfully" << std::endl;
}

void VulkanContext::CreateCommandPool()
{
  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = mQueueFamilyIndices.graphicsFamily.value(),
  };
  if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create command pool");
  }
}

void VulkanContext::CreateAllocator()
{
  VmaAllocatorCreateInfo allocatorInfo{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = mPhysicalDevice,
      .device = mDevice,
      .instance = mInstance,
  };

  if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create VMA allocator");
  }
}

VkCommandBuffer VulkanContext::BeginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = mCommandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkCommandBufferSubmitInfo cmdBufInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .commandBuffer = commandBuffer,
      .deviceMask = 0,
  };

  VkSubmitInfo2 submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmdBufInfo,
  };

  vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

  vkQueueWaitIdle(mGraphicsQueue);

  vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void VulkanContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferCopy copyRegion{
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };

  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}