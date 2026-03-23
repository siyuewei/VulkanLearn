#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace VkCommands
{
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             VkRenderPass renderPass,
                             VkFramebuffer framebuffer,
                             VkExtent2D swapChainExtent,
                             VkPipeline graphicsPipeline,
                             VkBuffer vertexBuffer,
                             VkBuffer indexBuffer,
                             VkPipelineLayout pipelineLayout,
                             VkDescriptorSet descriptorSet,
                             uint32_t indexCount);
}
