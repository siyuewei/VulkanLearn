#pragma once // 防止头文件重复包含

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace VkUtils
{
    struct VulkanBuffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void *mapped = nullptr;
        VkDeviceSize size = 0;
    };

    // 文件读取 (用于读 Shader 二进制文件)
    std::vector<char> readFile(const std::string &filename);

    // 查找内存类型 (比如找一块既能被 CPU 写，又在显卡上的内存)
    // 需要传入 physicalDevice 因为显存属性是物理设备的特征
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // 创建 Buffer 的全套流程 (创建 -> 查需求 -> 分配 -> 绑定)
    // 这是一个高度封装的函数
    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                      VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VulkanBuffer &outbuffer);

    // 开启一次性指令缓冲 (Helper: 开始录制)
    // 用于 copyBuffer, 以后还会用于 transitionImageLayout (纹理)
    VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

    // 结束一次性指令缓冲 (Helper: 提交并等待)
    void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);

    // 拷贝 Buffer (利用上面两个 Helper)
    void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
                    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    // 创建 Shader 模块 (可选，看你想不想把这个也移出来)
    VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
}