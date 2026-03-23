#pragma once

#include <string>

#include <vulkan/vulkan.h>

namespace VkPipelineUtils
{
    void createRenderPass(VkDevice device,
                          VkFormat swapChainImageFormat,
                          VkRenderPass &renderPass);

    void createDescriptorSetLayout(VkDevice device,
                                   VkDescriptorSetLayout &descriptorSetLayout);

    void createGraphicsPipeline(VkDevice device,
                                VkExtent2D swapChainExtent,
                                VkRenderPass renderPass,
                                VkDescriptorSetLayout descriptorSetLayout,
                                const std::string &vertShaderPath,
                                const std::string &fragShaderPath,
                                VkPipelineLayout &pipelineLayout,
                                VkPipeline &graphicsPipeline);

    void destroyPipelineResources(VkDevice device,
                                  VkDescriptorSetLayout &descriptorSetLayout,
                                  VkPipeline &graphicsPipeline,
                                  VkPipelineLayout &pipelineLayout,
                                  VkRenderPass &renderPass);
}
