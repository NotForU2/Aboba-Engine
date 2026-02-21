#include "VulkanSwapchain.hpp"

void VulkanSwapchain::Create(VulkanContext *context)
{
  CreateSwapchain(context);
  CreateImageViews(context);
}

void VulkanSwapchain::Destroy(const VulkanContext *context)
{
  for (auto imageView : mImageViews)
  {
    vkDestroyImageView(context->GetDevice(), imageView, nullptr);
  }
  mImageViews.clear();

  if (mSwapchain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(context->GetDevice(), mSwapchain, nullptr);
    mSwapchain = VK_NULL_HANDLE;
  }
}

void VulkanSwapchain::Recreate(VulkanContext *context)
{
  vkDeviceWaitIdle(context->GetDevice());
  Destroy(context);
  Create(context);
}

void VulkanSwapchain::CreateSwapchain(VulkanContext *context)
{
  SwapchainSupportDetails swapchainSupport = QuerySupport(context);

  VkSurfaceFormat2KHR surfaceFormat = ChooseSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode = ChoosePresentMode(swapchainSupport.presentModes);
  VkExtent2D extent = ChooseExtent(context, swapchainSupport.capabilities);

  uint32_t imageCount = swapchainSupport.capabilities.surfaceCapabilities.minImageCount + 1;

  if (swapchainSupport.capabilities.surfaceCapabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.surfaceCapabilities.maxImageCount)
  {
    imageCount = swapchainSupport.capabilities.surfaceCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = context->GetSurface(),
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
      context->GetGraphicsFamily(),
      context->GetPresentFamily(),
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

  if (vkCreateSwapchainKHR(context->GetDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Swapchain");
  }

  mImageFormat = surfaceFormat.surfaceFormat.format;
  mExtent = extent;

  vkGetSwapchainImagesKHR(context->GetDevice(), mSwapchain, &imageCount, nullptr);
  mImages.resize(imageCount);
  vkGetSwapchainImagesKHR(context->GetDevice(), mSwapchain, &imageCount, mImages.data());

  std::cout << "Swapchain created. Images: " << imageCount << ", Resolution: "
            << extent.width << "x" << extent.height << std::endl;
}

void VulkanSwapchain::CreateImageViews(VulkanContext *context)
{
  mImageViews.resize(mImages.size());

  for (size_t i = 0; i != mImages.size(); ++i)
  {
    VkImageViewCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = mImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = mImageFormat,
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

    if (vkCreateImageView(context->GetDevice(), &createInfo, nullptr, &mImageViews[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Image Views");
    }
  }
}

SwapchainSupportDetails VulkanSwapchain::QuerySupport(VulkanContext *context)
{
  SwapchainSupportDetails details;

  VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
      .surface = context->GetSurface(),
  };

  VkSurfaceCapabilities2KHR capabilities2{
      .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
  };
  vkGetPhysicalDeviceSurfaceCapabilities2KHR(context->GetPhysicalDevice(), &surfaceInfo2, &capabilities2);
  details.capabilities = capabilities2;

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormats2KHR(context->GetPhysicalDevice(), &surfaceInfo2, &formatCount, nullptr);
  if (formatCount != 0)
  {
    VkSurfaceFormat2KHR defaultFormat{
        .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
    };
    std::vector<VkSurfaceFormat2KHR> formats2(formatCount, defaultFormat);

    vkGetPhysicalDeviceSurfaceFormats2KHR(context->GetPhysicalDevice(), &surfaceInfo2, &formatCount, formats2.data());

    details.formats = formats2;
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(context->GetPhysicalDevice(), context->GetSurface(), &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->GetPhysicalDevice(), context->GetSurface(), &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormat2KHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats2)
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

VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
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

VkExtent2D VulkanSwapchain::ChooseExtent(VulkanContext *context, const VkSurfaceCapabilities2KHR &capabilities2)
{
  if (capabilities2.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities2.surfaceCapabilities.currentExtent;
  }
  else
  {
    int width = context->GetWindow()->GetFramebufferWidth();
    int height = context->GetWindow()->GetFramebufferHeight();

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