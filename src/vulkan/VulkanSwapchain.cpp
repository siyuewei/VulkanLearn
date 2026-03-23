#include "VulkanSwapchain.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace VkSwapchain
{
    void createSwapChain(VkPhysicalDevice physicalDevice,
                         VkDevice device,
                         VkSurfaceKHR surface,
                         uint32_t preferredWidth,
                         uint32_t preferredHeight,
                         VkSwapchainKHR &swapChain,
                         std::vector<VkImage> &swapChainImages,
                         VkFormat &swapChainImageFormat,
                         VkExtent2D &swapChainExtent)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount == 0)
        {
            throw std::runtime_error("当前设备没有可用的交换链表面格式!");
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        VkSurfaceFormatKHR surfaceFormat = formats[0];
        for (const auto &availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = availableFormat;
                break;
            }
        }

        VkExtent2D extent;
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            extent = capabilities.currentExtent;
        }
        else
        {
            extent = {preferredWidth, preferredHeight};
            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        {
            imageCount = capabilities.maxImageCount;
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount == 0)
        {
            throw std::runtime_error("当前设备没有可用的显示模式!");
        }

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto availablePresentMode : presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("无法创建交换链!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        std::cout << "交换链创建成功! 图片数量: " << imageCount << std::endl;
    }

    void createImageViews(VkDevice device,
                          const std::vector<VkImage> &swapChainImages,
                          VkFormat swapChainImageFormat,
                          std::vector<VkImageView> &swapChainImageViews)
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("无法创建图像视图!");
            }
        }

        std::cout << "图像视图创建成功!" << std::endl;
    }

    void createFramebuffers(VkDevice device,
                            VkRenderPass renderPass,
                            const std::vector<VkImageView> &swapChainImageViews,
                            VkExtent2D swapChainExtent,
                            std::vector<VkFramebuffer> &swapChainFramebuffers)
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                swapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("无法创建帧缓冲区!");
            }
        }

        std::cout << "帧缓冲区创建成功!" << std::endl;
    }

    void destroyFramebuffers(VkDevice device,
                             std::vector<VkFramebuffer> &swapChainFramebuffers)
    {
        for (auto &framebuffer : swapChainFramebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
                framebuffer = VK_NULL_HANDLE;
            }
        }
        swapChainFramebuffers.clear();
    }

    void destroyImageViews(VkDevice device,
                           std::vector<VkImageView> &swapChainImageViews)
    {
        for (auto &imageView : swapChainImageViews)
        {
            if (imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device, imageView, nullptr);
                imageView = VK_NULL_HANDLE;
            }
        }
        swapChainImageViews.clear();
    }

    void destroySwapChain(VkDevice device,
                          VkSwapchainKHR &swapChain,
                          std::vector<VkImage> &swapChainImages)
    {
        if (swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
            swapChain = VK_NULL_HANDLE;
        }
        swapChainImages.clear();
    }
}
