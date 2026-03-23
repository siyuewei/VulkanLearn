//
// Created by 15006 on 2025/12/22.
//

#ifndef VULKANLEARN_VULKANLEARNAPPLICATION_H
#define VULKANLEARN_VULKANLEARNAPPLICATION_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdexcept> // 用于抛出异常
#include <vector>
#include <array>

#include "VulkanUtils.h"

class VulkanLearnApplication {
public:
    void run();

private:
    uint32_t currentFrame = 0;

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

    VkUtils::VulkanBuffer vertexBuffer;
    VkUtils::VulkanBuffer indexBuffer;
    std::vector<VkUtils::VulkanBuffer> uniformBuffers;

    // 描述符 (将 Buffer 连接到管线的接口)
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // ==========================================
    // 5. 逐帧控制 (Per-Frame Objects)
    // 用于控制渲染循环的同步与调度
    // ==========================================
    std::vector<VkCommandBuffer> commandBuffers;

    // 同步对象 (红绿灯/栅栏)
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    //6.验证层
    VkDebugUtilsMessengerEXT debugMessenger;

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

    int findQueueFamilies(VkPhysicalDevice candidatePhysicalDevice);

    void mainloop();
    void drawFrame();

    void setupDebugMessenger();
    bool checkValidationLayerSupport();
};

#endif //VULKANLEARN_VULKANLEARNAPPLICATION_H
