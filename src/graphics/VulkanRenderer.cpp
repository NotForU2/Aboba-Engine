#include "VulkanRenderer.hpp"

void VulkanRenderer::Init(SDL_Window *window)
{
  mWindow = window;
  CreateInstance();
}

void VulkanRenderer::Cleanup()
{
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
    throw std::runtime_error("Validation layers requested, but not available.");
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
      .apiVersion = targetVersion};

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

  // В Vulkan 1.3+ для переносимости (особенно на macOS через MoltenVK) может понадобиться флаг (flag)
  // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR, но на Windows пока опустим.
  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
      .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()};

  if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Vulkan Instance");
  }

  std::cout << "Vulkan Instance created successfully" << std::endl;
}