#include "VulkanResources.h"

#include <cstring>
#include <stdexcept>

namespace VkResources
{
    void createCommandPool(VkDevice device,
                           int queueFamilyIndex,
                           VkCommandPool &commandPool)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex);

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("无法创建命令池!");
        }
    }

    void createVertexBuffer(VkDevice device,
                            VkPhysicalDevice physicalDevice,
                            VkCommandPool commandPool,
                            VkQueue graphicsQueue,
                            const std::vector<VkSceneData::Vertex> &vertices,
                            VkUtils::VulkanBuffer &vertexBuffer)
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkUtils::VulkanBuffer stagingBuffer;
        VkUtils::createBuffer(device, physicalDevice, bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuffer);

        void *data = nullptr;
        vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
        std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device, stagingBuffer.memory);

        VkUtils::createBuffer(device, physicalDevice, bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              vertexBuffer);

        VkUtils::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
        vkFreeMemory(device, stagingBuffer.memory, nullptr);
    }

    void createIndexBuffer(VkDevice device,
                           VkPhysicalDevice physicalDevice,
                           VkCommandPool commandPool,
                           VkQueue graphicsQueue,
                           const std::vector<uint16_t> &indices,
                           VkUtils::VulkanBuffer &indexBuffer)
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkUtils::VulkanBuffer stagingBuffer;
        VkUtils::createBuffer(device, physicalDevice, bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuffer);

        void *data = nullptr;
        vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
        std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device, stagingBuffer.memory);

        VkUtils::createBuffer(device, physicalDevice, bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              indexBuffer);

        VkUtils::copyBuffer(device, commandPool, graphicsQueue, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
        vkFreeMemory(device, stagingBuffer.memory, nullptr);
    }

    void createUniformBuffers(VkDevice device,
                              VkPhysicalDevice physicalDevice,
                              uint32_t frameCount,
                              VkDeviceSize bufferSize,
                              std::vector<VkUtils::VulkanBuffer> &uniformBuffers)
    {
        uniformBuffers.resize(frameCount);

        for (uint32_t i = 0; i < frameCount; i++)
        {
            VkUtils::createBuffer(device, physicalDevice, bufferSize,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  uniformBuffers[i]);

            vkMapMemory(device, uniformBuffers[i].memory, 0, bufferSize, 0, &uniformBuffers[i].mapped);
        }
    }

    void createDescriptorPool(VkDevice device,
                              uint32_t frameCount,
                              VkDescriptorPool &descriptorPool)
    {
        VkDescriptorPoolSize descriptorPoolSize{};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSize.descriptorCount = frameCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &descriptorPoolSize;
        poolInfo.maxSets = frameCount;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void createDescriptorSets(VkDevice device,
                              uint32_t frameCount,
                              VkDescriptorPool descriptorPool,
                              VkDescriptorSetLayout descriptorSetLayout,
                              const std::vector<VkUtils::VulkanBuffer> &uniformBuffers,
                              VkDeviceSize uniformRange,
                              std::vector<VkDescriptorSet> &descriptorSets)
    {
        std::vector<VkDescriptorSetLayout> layouts(frameCount, descriptorSetLayout);

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = frameCount;
        descriptorSetAllocateInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(frameCount);
        if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        for (uint32_t i = 0; i < frameCount; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = uniformRange;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void createCommandBuffers(VkDevice device,
                              VkCommandPool commandPool,
                              uint32_t frameCount,
                              std::vector<VkCommandBuffer> &commandBuffers)
    {
        commandBuffers.resize(frameCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = frameCount;

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("无法分配命令缓冲!");
        }
    }

    void createSyncObjects(VkDevice device,
                           uint32_t frameCount,
                           std::vector<VkSemaphore> &imageAvailableSemaphores,
                           std::vector<VkSemaphore> &renderFinishedSemaphores,
                           std::vector<VkFence> &inFlightFences)
    {
        imageAvailableSemaphores.resize(frameCount);
        renderFinishedSemaphores.resize(frameCount);
        inFlightFences.resize(frameCount);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < frameCount; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("无法创建同步对象!");
            }
        }
    }

    void destroySyncObjects(VkDevice device,
                            std::vector<VkSemaphore> &imageAvailableSemaphores,
                            std::vector<VkSemaphore> &renderFinishedSemaphores,
                            std::vector<VkFence> &inFlightFences)
    {
        for (auto &semaphore : renderFinishedSemaphores)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device, semaphore, nullptr);
                semaphore = VK_NULL_HANDLE;
            }
        }

        for (auto &semaphore : imageAvailableSemaphores)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device, semaphore, nullptr);
                semaphore = VK_NULL_HANDLE;
            }
        }

        for (auto &fence : inFlightFences)
        {
            if (fence != VK_NULL_HANDLE)
            {
                vkDestroyFence(device, fence, nullptr);
                fence = VK_NULL_HANDLE;
            }
        }

        renderFinishedSemaphores.clear();
        imageAvailableSemaphores.clear();
        inFlightFences.clear();
    }

    void destroyDescriptorPool(VkDevice device,
                               VkDescriptorPool &descriptorPool,
                               std::vector<VkDescriptorSet> &descriptorSets)
    {
        if (descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
            descriptorPool = VK_NULL_HANDLE;
        }
        descriptorSets.clear();
    }

    void destroyUniformBuffers(VkDevice device,
                               std::vector<VkUtils::VulkanBuffer> &uniformBuffers)
    {
        for (auto &uniformBuffer : uniformBuffers)
        {
            destroyBuffer(device, uniformBuffer);
        }
        uniformBuffers.clear();
    }

    void destroyBuffer(VkDevice device,
                       VkUtils::VulkanBuffer &buffer)
    {
        if (buffer.buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, buffer.buffer, nullptr);
            buffer.buffer = VK_NULL_HANDLE;
        }
        if (buffer.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, buffer.memory, nullptr);
            buffer.memory = VK_NULL_HANDLE;
        }
        buffer.mapped = nullptr;
        buffer.size = 0;
    }

    void destroyCommandPool(VkDevice device,
                            VkCommandPool &commandPool,
                            std::vector<VkCommandBuffer> &commandBuffers)
    {
        if (commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, commandPool, nullptr);
            commandPool = VK_NULL_HANDLE;
        }
        commandBuffers.clear();
    }
}
