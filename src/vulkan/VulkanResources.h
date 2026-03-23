#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanSceneData.h"
#include "VulkanUtils.h"

namespace VkResources
{
    void createCommandPool(VkDevice device,
                           int queueFamilyIndex,
                           VkCommandPool &commandPool);

    void createVertexBuffer(VkDevice device,
                            VkPhysicalDevice physicalDevice,
                            VkCommandPool commandPool,
                            VkQueue graphicsQueue,
                            const std::vector<VkSceneData::Vertex> &vertices,
                            VkUtils::VulkanBuffer &vertexBuffer);

    void createIndexBuffer(VkDevice device,
                           VkPhysicalDevice physicalDevice,
                           VkCommandPool commandPool,
                           VkQueue graphicsQueue,
                           const std::vector<uint16_t> &indices,
                           VkUtils::VulkanBuffer &indexBuffer);

    void createUniformBuffers(VkDevice device,
                              VkPhysicalDevice physicalDevice,
                              uint32_t frameCount,
                              VkDeviceSize bufferSize,
                              std::vector<VkUtils::VulkanBuffer> &uniformBuffers);

    void createDescriptorPool(VkDevice device,
                              uint32_t frameCount,
                              VkDescriptorPool &descriptorPool);

    void createDescriptorSets(VkDevice device,
                              uint32_t frameCount,
                              VkDescriptorPool descriptorPool,
                              VkDescriptorSetLayout descriptorSetLayout,
                              const std::vector<VkUtils::VulkanBuffer> &uniformBuffers,
                              VkDeviceSize uniformRange,
                              std::vector<VkDescriptorSet> &descriptorSets);

    void createCommandBuffers(VkDevice device,
                              VkCommandPool commandPool,
                              uint32_t frameCount,
                              std::vector<VkCommandBuffer> &commandBuffers);

    void createSyncObjects(VkDevice device,
                           uint32_t frameCount,
                           std::vector<VkSemaphore> &imageAvailableSemaphores,
                           std::vector<VkSemaphore> &renderFinishedSemaphores,
                           std::vector<VkFence> &inFlightFences);

    void destroySyncObjects(VkDevice device,
                            std::vector<VkSemaphore> &imageAvailableSemaphores,
                            std::vector<VkSemaphore> &renderFinishedSemaphores,
                            std::vector<VkFence> &inFlightFences);

    void destroyDescriptorPool(VkDevice device,
                               VkDescriptorPool &descriptorPool,
                               std::vector<VkDescriptorSet> &descriptorSets);

    void destroyUniformBuffers(VkDevice device,
                               std::vector<VkUtils::VulkanBuffer> &uniformBuffers);

    void destroyBuffer(VkDevice device,
                       VkUtils::VulkanBuffer &buffer);

    void destroyCommandPool(VkDevice device,
                            VkCommandPool &commandPool,
                            std::vector<VkCommandBuffer> &commandBuffers);
}
