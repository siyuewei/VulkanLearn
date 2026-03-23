#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace VkCore
{
    VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger);

    void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       VkAllocationCallbacks *pAllocator);

    bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

    void createInstance(bool enableValidationLayers,
                        const std::vector<const char *> &validationLayers,
                        VkInstance &instance);

    void setupDebugMessenger(VkInstance instance,
                             bool enableValidationLayers,
                             VkDebugUtilsMessengerEXT &debugMessenger);

    void createSurface(VkInstance instance, GLFWwindow *window, VkSurfaceKHR &surface);

    void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice &physicalDevice);

    int findQueueFamily(VkPhysicalDevice candidatePhysicalDevice, VkSurfaceKHR surface);

    void createLogicalDevice(VkPhysicalDevice physicalDevice,
                             int graphicsQueueFamilyIndex,
                             VkDevice &device,
                             VkQueue &graphicsQueue,
                             VkQueue &presentQueue);

    void destroyLogicalDevice(VkDevice &device);

    void destroySurface(VkInstance instance, VkSurfaceKHR &surface);

    void destroyInstance(VkInstance &instance);
}
