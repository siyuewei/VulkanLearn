//
// Created by 15006 on 2025/12/22.
//

#include "VulkanLearnApplication.h"

#include "VulkanCommands.h"
#include "VulkanCore.h"
#include "VulkanPipeline.h"
#include "VulkanResources.h"
#include "VulkanSceneData.h"
#include "VulkanSwapchain.h"
#include "ObjMeshLoader.h"

#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
#ifdef NDEBUG
    constexpr bool kEnableValidationLayers = false;
#else
    constexpr bool kEnableValidationLayers = true;
#endif

    constexpr uint32_t kWidth = 800;
    constexpr uint32_t kHeight = 600;
    constexpr uint32_t kFrameCount = static_cast<uint32_t>(VkSceneData::kMaxFramesInFlight);

    const std::vector<const char *> kValidationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    std::string ShaderPath(const char *filename)
    {
#ifdef SHADER_DIR
        static const std::string base = SHADER_DIR;
#else
        static const std::string base = "shaders";
#endif
        return base + "/" + filename;
    }

    std::string MeshPath()
    {
#ifdef ASSET_DIR
        static const std::string base = ASSET_DIR;
#else
        static const std::string base = "assets";
#endif
        return base + "/models/tumbler model/tumbler.obj";
    }
} // namespace

void VulkanLearnApplication::run()
{
    initWindow();
    initVulkan();
    mainloop();
    cleanup();
}

void VulkanLearnApplication::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(kWidth, kHeight, "Vulkan Refactored", nullptr, nullptr);
    if (!window)
    {
        throw std::runtime_error("无法创建 GLFW 窗口!");
    }
}

void VulkanLearnApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createImageViews();

    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();

    createFramebuffers();

    createCommandPool();

    loadMeshData();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();

    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffer();
    createSyncObjects();
}

void VulkanLearnApplication::cleanup()
{
    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

    if (device != VK_NULL_HANDLE)
    {
        VkResources::destroySyncObjects(device,
                                        imageAvailableSemaphores,
                                        renderFinishedSemaphores,
                                        inFlightFences);

        VkResources::destroyDescriptorPool(device,
                                           descriptorPool,
                                           descriptorSets);

        VkResources::destroyUniformBuffers(device, uniformBuffers);
        VkResources::destroyBuffer(device, indexBuffer);
        VkResources::destroyBuffer(device, vertexBuffer);
        VkResources::destroyCommandPool(device, commandPool, commandBuffers);

        VkSwapchain::destroyFramebuffers(device, swapChainFramebuffers);
        VkPipelineUtils::destroyPipelineResources(device,
                                                  descriptorSetLayout,
                                                  graphicsPipeline,
                                                  pipelineLayout,
                                                  renderPass);
        VkSwapchain::destroyImageViews(device, swapChainImageViews);
        VkSwapchain::destroySwapChain(device, swapChain, swapChainImages);
    }

    if (kEnableValidationLayers && debugMessenger != VK_NULL_HANDLE)
    {
        VkCore::destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
    }

    VkCore::destroyLogicalDevice(device);
    VkCore::destroySurface(instance, surface);
    VkCore::destroyInstance(instance);
    physicalDevice = VK_NULL_HANDLE;
    graphicsQueue = VK_NULL_HANDLE;
    presentQueue = VK_NULL_HANDLE;
    swapChainImageFormat = VK_FORMAT_UNDEFINED;
    swapChainExtent = {};
    currentFrame = 0;
    activeIndexCount = 0;
    meshVertices.clear();
    meshIndices.clear();

    if (window != nullptr)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    std::cout << "资源已清理。" << std::endl;
}

void VulkanLearnApplication::createInstance()
{
    VkCore::createInstance(kEnableValidationLayers, kValidationLayers, instance);
}

void VulkanLearnApplication::createSurface()
{
    VkCore::createSurface(instance, window, surface);
}

void VulkanLearnApplication::pickPhysicalDevice()
{
    VkCore::pickPhysicalDevice(instance, physicalDevice);
}

void VulkanLearnApplication::createLogicalDevice()
{
    const int graphicsQueueFamilyIndex = findQueueFamilies(physicalDevice);
    VkCore::createLogicalDevice(physicalDevice, graphicsQueueFamilyIndex, device, graphicsQueue, presentQueue);
}

void VulkanLearnApplication::createSwapChain()
{
    VkSwapchain::createSwapChain(physicalDevice,
                                 device,
                                 surface,
                                 kWidth,
                                 kHeight,
                                 swapChain,
                                 swapChainImages,
                                 swapChainImageFormat,
                                 swapChainExtent);
}

void VulkanLearnApplication::createImageViews()
{
    VkSwapchain::createImageViews(device,
                                  swapChainImages,
                                  swapChainImageFormat,
                                  swapChainImageViews);
}

void VulkanLearnApplication::createRenderPass()
{
    VkPipelineUtils::createRenderPass(device, swapChainImageFormat, renderPass);
}

void VulkanLearnApplication::createGraphicsPipeline()
{
    VkPipelineUtils::createGraphicsPipeline(device,
                                            swapChainExtent,
                                            renderPass,
                                            descriptorSetLayout,
                                            ShaderPath("vert.spv"),
                                            ShaderPath("frag.spv"),
                                            pipelineLayout,
                                            graphicsPipeline);
}

void VulkanLearnApplication::createFramebuffers()
{
    VkSwapchain::createFramebuffers(device,
                                    renderPass,
                                    swapChainImageViews,
                                    swapChainExtent,
                                    swapChainFramebuffers);
}

void VulkanLearnApplication::createCommandPool()
{
    VkResources::createCommandPool(device,
                                   findQueueFamilies(physicalDevice),
                                   commandPool);
}

void VulkanLearnApplication::createVertexBuffer()
{
    VkResources::createVertexBuffer(device,
                                    physicalDevice,
                                    commandPool,
                                    graphicsQueue,
                                    meshVertices,
                                    vertexBuffer);
}

void VulkanLearnApplication::createIndexBuffer()
{
    VkResources::createIndexBuffer(device,
                                   physicalDevice,
                                   commandPool,
                                   graphicsQueue,
                                   meshIndices,
                                   indexBuffer);
}

void VulkanLearnApplication::loadMeshData()
{
    const std::string meshPath = MeshPath();
    Mesh::MeshAsset loadedMesh;
    std::string loadError;

    if (!Mesh::ObjMeshLoader::loadFromFile(meshPath, loadedMesh, loadError))
    {
        std::cout << "Mesh 加载失败，使用内置立方体: " << loadError << std::endl;
        meshVertices = VkSceneData::kVertices;
        meshIndices = VkSceneData::kIndices;
        activeIndexCount = static_cast<uint32_t>(meshIndices.size());
        return;
    }

    if (loadedMesh.indices.size() > std::numeric_limits<uint16_t>::max())
    {
        std::cout << "Mesh 索引超过 uint16 上限，使用内置立方体。" << std::endl;
        meshVertices = VkSceneData::kVertices;
        meshIndices = VkSceneData::kIndices;
        activeIndexCount = static_cast<uint32_t>(meshIndices.size());
        return;
    }

    meshVertices.clear();
    meshIndices.clear();
    meshVertices.reserve(loadedMesh.positions.size());
    meshIndices.reserve(loadedMesh.indices.size());

    for (size_t i = 0; i < loadedMesh.positions.size(); i++)
    {
        const glm::vec3 position = loadedMesh.positions[i];
        const glm::vec3 normal = loadedMesh.hasNormals() ? loadedMesh.normals[i] : glm::vec3(0.0f, 0.0f, 1.0f);
        const glm::vec3 color = (normal * 0.5f) + glm::vec3(0.5f);
        meshVertices.push_back({position, color});
    }

    for (const uint32_t index : loadedMesh.indices)
    {
        meshIndices.push_back(static_cast<uint16_t>(index));
    }

    activeIndexCount = static_cast<uint32_t>(meshIndices.size());

    std::cout << "Mesh 路径: " << meshPath << std::endl;
    std::cout << "Mesh 加载成功: vertices=" << meshVertices.size()
              << ", indices=" << meshIndices.size() << std::endl;
}

void VulkanLearnApplication::createCommandBuffer()
{
    VkResources::createCommandBuffers(device,
                                      commandPool,
                                      kFrameCount,
                                      commandBuffers);
}

void VulkanLearnApplication::createSyncObjects()
{
    VkResources::createSyncObjects(device,
                                   kFrameCount,
                                   imageAvailableSemaphores,
                                   renderFinishedSemaphores,
                                   inFlightFences);
}

void VulkanLearnApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommands::recordCommandBuffer(commandBuffer,
                                    renderPass,
                                    swapChainFramebuffers[imageIndex],
                                    swapChainExtent,
                                    graphicsPipeline,
                                    vertexBuffer.buffer,
                                    indexBuffer.buffer,
                                    pipelineLayout,
                                    descriptorSets[currentFrame],
                                    activeIndexCount);
}

void VulkanLearnApplication::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    VkSceneData::UniformBufferObject ubo{};

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, time * glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    float scale = 1.0f + 0.2f * std::sin(time * 2.0f);
    model = glm::scale(model, glm::vec3(scale));
    ubo.model = model;

    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f),
                                      swapChainExtent.width / static_cast<float>(swapChainExtent.height),
                                      0.1f,
                                      10.0f);
    ubo.projection[1][1] *= -1;

    std::memcpy(uniformBuffers[currentImage].mapped, &ubo, sizeof(VkSceneData::UniformBufferObject));
}

void VulkanLearnApplication::createDescriptorSetLayout()
{
    VkPipelineUtils::createDescriptorSetLayout(device, descriptorSetLayout);
}

void VulkanLearnApplication::createUniformBuffers()
{
    VkResources::createUniformBuffers(device,
                                      physicalDevice,
                                      kFrameCount,
                                      sizeof(VkSceneData::UniformBufferObject),
                                      uniformBuffers);
}

void VulkanLearnApplication::createDescriptorPool()
{
    VkResources::createDescriptorPool(device,
                                      kFrameCount,
                                      descriptorPool);
}

void VulkanLearnApplication::createDescriptorSets()
{
    VkResources::createDescriptorSets(device,
                                      kFrameCount,
                                      descriptorPool,
                                      descriptorSetLayout,
                                      uniformBuffers,
                                      sizeof(VkSceneData::UniformBufferObject),
                                      descriptorSets);
}

int VulkanLearnApplication::findQueueFamilies(VkPhysicalDevice candidatePhysicalDevice)
{
    return VkCore::findQueueFamily(candidatePhysicalDevice, surface);
}

void VulkanLearnApplication::mainloop()
{
    std::cout << "进入主循环..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void VulkanLearnApplication::drawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    const uint32_t imageIndex = acquireNextImageForFrame();
    submitCurrentFrame(imageIndex);
    presentFrame(imageIndex);

    currentFrame = (currentFrame + 1) % kFrameCount;
}

uint32_t VulkanLearnApplication::acquireNextImageForFrame()
{
    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device,
                                            swapChain,
                                            UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("无法获取交换链图像!");
    }

    return imageIndex;
}

void VulkanLearnApplication::submitCurrentFrame(uint32_t imageIndex)
{
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    updateUniformBuffer(currentFrame);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("无法提交绘制命令!");
    }
}

void VulkanLearnApplication::presentFrame(uint32_t imageIndex)
{
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}

void VulkanLearnApplication::setupDebugMessenger()
{
    VkCore::setupDebugMessenger(instance, kEnableValidationLayers, debugMessenger);
}
