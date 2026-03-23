#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

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
                         VkExtent2D &swapChainExtent);

    void createImageViews(VkDevice device,
                          const std::vector<VkImage> &swapChainImages,
                          VkFormat swapChainImageFormat,
                          std::vector<VkImageView> &swapChainImageViews);

    void createFramebuffers(VkDevice device,
                            VkRenderPass renderPass,
                            const std::vector<VkImageView> &swapChainImageViews,
                            VkExtent2D swapChainExtent,
                            std::vector<VkFramebuffer> &swapChainFramebuffers);

    void destroyFramebuffers(VkDevice device,
                             std::vector<VkFramebuffer> &swapChainFramebuffers);

    void destroyImageViews(VkDevice device,
                           std::vector<VkImageView> &swapChainImageViews);

    void destroySwapChain(VkDevice device,
                          VkSwapchainKHR &swapChain,
                          std::vector<VkImage> &swapChainImages);
}
