//
// Created by 15006 on 2025/12/22.
//

#ifndef VULKANLEARN_VULKANLEARNAPPLICATION_H
#define VULKANLEARN_VULKANLEARNAPPLICATION_H

#define GLM_FORCE_RADIANS //GLM使用弧度制
#define GLM_FORCE_DEPTH_ZERO_TO_ONE //深度范围0-1（OpenGL是-1到1）
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept> // 用于抛出异常
#include <vector>
#include <fstream>
#include <algorithm> // 用于 std::min/max
#include <windows.h> // 用于控制台乱码修复
#include <array>

class VulkanLearnApplication {
public:
    void run();

private:
    GLFWwindow* window;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理显卡 (不需销毁)
    VkDevice device;                                  // 逻辑设备
    VkQueue graphicsQueue;                            // 图形队列
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers; // 对应每一个交换链图像的帧缓冲

    VkCommandPool commandPool;                     // 命令池
    VkCommandBuffer commandBuffer;               // 命令缓冲
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkSemaphore imageAvailableSemaphore; // 图像可用信号量
    VkSemaphore renderFinishedSemaphore;  // 渲染完成信号量
    VkFence inFlightFence;            // 帧内同步栅栏

    VkDescriptorSetLayout descriptorSetLayout; //插座的设计图

    void initWindow();
    void initVulkan();
    void cleanup();

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();

    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createVertexBuffer();
    void createIndexBuffer();
    void createSyncObjects();
    void createDescriptorSetLayout();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    int findQueueFamilies(VkPhysicalDevice device);
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,VkBuffer& buffer,VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void mainloop();
    void drawFrame();

};


#endif //VULKANLEARN_VULKANLEARNAPPLICATION_H