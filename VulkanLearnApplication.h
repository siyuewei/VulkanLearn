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
private:
    // ==========================================
    // 1. 核心与窗口 (Core)
    // 整个程序的根基，最后销毁
    // ==========================================
    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue; // 建议加上显示队列，虽然通常和图形队列是同一个

    // ==========================================
    // 2. 交换链与画布 (Swapchain)
    // 跟屏幕尺寸有关，窗口调整大小时需要重建的部分
    // ==========================================
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers; // 画板

    // ==========================================
    // 3. 渲染管线 (Pipeline)
    // 定义了“怎么画”。通常不可变，要改就得重建
    // ==========================================
    VkRenderPass renderPass;       // 宏观流程
    VkDescriptorSetLayout descriptorSetLayout; // 插座设计图 (新增位置)
    VkPipelineLayout pipelineLayout; // 管线布局 (插座 + 推送常量)
    VkPipeline graphicsPipeline;   // 具体的绘画机器

    // ==========================================
    // 4. 资源数据 (Resources)
    // 顶点、索引、纹理等原材料
    // ==========================================
    VkCommandPool commandPool;     // 指令内存池

    // 顶点数据
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    // Uniform 数据 (每一帧都有独立的一份)
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    // 描述符 (将 Buffer 连接到管线的接口)
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // ==========================================
    // 5. 逐帧控制 (Per-Frame Objects)
    // 用于控制渲染循环的同步与调度
    // ==========================================
    VkCommandBuffer commandBuffer; // 现在的写法只有一个，后面会改成数组

    // 同步对象 (红绿灯/栅栏)
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

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
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

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
