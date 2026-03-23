#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace VkSceneData
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 model;
    };

    constexpr int kMaxFramesInFlight = 3;

    extern const std::vector<Vertex> kVertices;
    extern const std::vector<uint16_t> kIndices;
}
