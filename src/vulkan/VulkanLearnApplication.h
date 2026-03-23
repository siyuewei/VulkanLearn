//
// Created by 15006 on 2025/12/22.
//

#ifndef VULKANLEARN_VULKANLEARNAPPLICATION_H
#define VULKANLEARN_VULKANLEARNAPPLICATION_H

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "VulkanUtils.h"

class VulkanLearnApplication
{
public:
    void run();

private:
    uint32_t currentFrame = 0;

    // ==========================================
    // 1. 核心与窗口 (Core)
    // 整个程序的根基，最后销毁
    // ==========================================
    GLFWwindow *window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    // ==========================================
    // 2. 交换链与画布 (Swapchain)
    // 跟屏幕尺寸有关，窗口调整大小时需要重建的部分
    // ==========================================
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers; // 画板

    // ==========================================
    // 3. 渲染管线 (Pipeline)
    // 定义了“怎么画”。通常不可变，要改就得重建
    // ==========================================
    VkRenderPass renderPass = VK_NULL_HANDLE;                   // 宏观流程
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE; // 插座设计图 (新增位置)
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;           // 管线布局 (插座 + 推送常量)
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;               // 具体的绘画机器

    // ==========================================
    // 4. 资源数据 (Resources)
    // 顶点、索引、纹理等原材料
    // ==========================================
    VkCommandPool commandPool = VK_NULL_HANDLE; // 指令内存池

    VkUtils::VulkanBuffer vertexBuffer;
    VkUtils::VulkanBuffer indexBuffer;
    std::vector<VkUtils::VulkanBuffer> uniformBuffers;

    // 描述符 (将 Buffer 连接到管线的接口)
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
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

    // 6.验证层
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

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
    uint32_t acquireNextImageForFrame();
    void submitCurrentFrame(uint32_t imageIndex);
    void presentFrame(uint32_t imageIndex);

    void setupDebugMessenger();
};

#endif // VULKANLEARN_VULKANLEARNAPPLICATION_H
