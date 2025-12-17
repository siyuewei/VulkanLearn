#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept> // 用于抛出异常
#include <vector>
#include <fstream>
#include <algorithm> // 用于 std::min/max
#include <windows.h> // 用于控制台乱码修复

// 窗口大小常量
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
public:
    // 对外唯一的接口：启动程序
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // ==================================================
    // 1. 成员变量 (这是我们的核心资产，存活于整个程序生命周期)
    // ==================================================
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

    // ==================================================
    // 2. 初始化流程 (分步骤实现)
    // ==================================================

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 禁止 OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // 禁止缩放
        
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Refactored", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("无法创建 GLFW 窗口!");
        }
    }

    void initVulkan() {
        //以此调用各个子系统的初始化函数
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
    }

    // --- 步骤 2.1: 创建 Instance ---
    void createInstance() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("无法创建 Vulkan Instance!");
        }
    }

    // --- 步骤 2.2: 创建 Surface ---
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("无法创建 Window Surface!");
        }
    }

    // --- 步骤 2.3: 挑选物理显卡 ---
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) throw std::runtime_error("找不到支持 Vulkan 的 GPU!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // 简单策略：优先选独显，否则选第一个
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                physicalDevice = device;
                std::cout << "已选中独立显卡: " << props.deviceName << std::endl;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            physicalDevice = devices[0];
            std::cout << "未找到独显，使用默认设备。" << std::endl;
        }
    }

    // --- 步骤 2.4: 创建逻辑设备 & 获取队列 ---
    void createLogicalDevice() {
        // 1. 找队列族索引
        int indices = findQueueFamilies(physicalDevice);

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // 2. 准备设备扩展 (交换链需要)
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("无法创建逻辑设备!");
        }

        // 3. 获取队列句柄
        vkGetDeviceQueue(device, indices, 0, &graphicsQueue);
    }

    // --- 步骤 2.5: 创建交换链 ---
    void createSwapChain() {
        // 查询支持的细节
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        // 1. 选格式 (Format)
        VkSurfaceFormatKHR surfaceFormat = formats[0];
        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                break;
            }
        }

        // 2. 选分辨率 (Extent)
        VkExtent2D extent;
        if (capabilities.currentExtent.width != UINT32_MAX) {
            extent = capabilities.currentExtent;
        } else {
            extent = {WIDTH, HEIGHT};
            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        // 3. 选图片数量
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // 强制开启 V-Sync
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("无法创建交换链!");
        }

        // 4. 保存结果到成员变量
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        std::cout << "交换链创建成功! 图片数量: " << imageCount << std::endl;
    }

    // --- 步骤 2.6: 创建图像视图 ---
    void createImageViews(){
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("无法创建图像视图!");
            }
        }
        std::cout << "图像视图创建成功!" << std::endl;
    }

    // --- 步骤 2.6.5： 创建渲染流程（Render Pass）---
    void createRenderPass() {
        //1.颜色附件描述（Attachment Description）
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat; //与交换链图像格式相同
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //不使用多重采样
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //渲染开始前清除
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //渲染结束后存储

        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //不使用模板缓冲
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //不

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //不关心初始布局
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //呈现给屏

        //2.子流程引用（Subpass Reference）
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0; //引用第一个附件
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //颜色附件布局

        //3.子流程描述（Subpass Description）
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //图形管
        subpass.colorAttachmentCount = 1; //一个颜色附件
        subpass.pColorAttachments = &colorAttachmentRef; //引用颜色附件

        //4.创建Render Pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1; //一个附件
        renderPassInfo.pAttachments = &colorAttachment; //附件描述
        renderPassInfo.subpassCount = 1; //一个子流程
        renderPassInfo.pSubpasses = &subpass; //子流程描述

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("无法创建渲染流程!");
        }
        std::cout << "渲染流程创建成功!" << std::endl;
    }

    // --- 步骤 2.7: 创建着色器模块 ---
    void createGraphicsPipeline(){
        //A.加载着色器
        auto vertShaderCode = readFile("D:/0-wsy/code/VulkanLearn/shaders/vert.spv");
        auto fragShaderCode = readFile("D:/0-wsy/code/VulkanLearn/shaders/frag.spv");

        std::cout << "着色器代码大小: 顶点着色器 " << vertShaderCode.size() 
                  << " 字节, 片段着色器 " << fragShaderCode.size() << " 字节." << std::endl;

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        //B.配置着色器
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main"; //入口函数名称

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main"; //入口函数名称

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        //C.顶点输入
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0; //无顶点数据
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        //D.输入装配
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //三角形列表
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        //E.视口和裁剪
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        //F.光栅化
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //填充模式
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //背面剔除
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //顺时针为正面
        rasterizer.depthBiasEnable = VK_FALSE;

        //G.多重采样 (暂时关闭)
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        //H.颜色混合
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        //I.管线布局
        // 用于传递 Uniform 变量 (目前没有)
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; //无描述符集
        pipelineLayoutInfo.pushConstantRangeCount = 0; //无推送常量

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("无法创建管线布局!");
        }

        //J.创建图形管线
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; //无深度模板缓冲
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; //无动态状态

        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("无法创建图形管线!");
        }

        std::cout << "图形管线创建成功!" << std::endl;

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    // --- 辅助函数：查找队列族 ---
    int findQueueFamilies(VkPhysicalDevice device) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < queueFamilies.size(); i++) {
            // 我们同时需要: 支持图形(Graphics) 且 支持显示(Present)
            // 简单起见，这里假设同一个队列族同时支持这两者 (绝大多数显卡都是这样)
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                return i;
            }
        }
        throw std::runtime_error("找不到合适的队列族!");
    }

    // 辅助函数： 读取二进制文件
    static std::vector<char> readFile(const std::string& filename){
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()){
            throw std::runtime_error("无法打开文件: " + filename);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("无法创建着色器模块!");
        }
        return shaderModule;
    }

    // ==================================================
    // 3. 主循环
    // ==================================================
    void mainLoop() {
        std::cout << "进入主循环..." << std::endl;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            // drawFrame(); // 未来在这里画画
        }
    }

    // ==================================================
    // 4. 清理资源 (注意顺序：与创建顺序相反)
    // ==================================================
    void cleanup() {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);       // 1. 销毁管线
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);   // 2. 销毁布局
        vkDestroyRenderPass(device, renderPass, nullptr);           // 3. 销毁 Render Pass
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "资源已清理。" << std::endl;
    }
};

int main() {
    SetConsoleOutputCP(65001); // 修复控制台乱码

    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}