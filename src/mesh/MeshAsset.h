#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace Mesh
{
    struct Bounds
    {
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

        void expand(const glm::vec3 &p)
        {
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        bool isValid() const
        {
            return min.x <= max.x && min.y <= max.y && min.z <= max.z;
        }
    };

    struct MeshAsset
    {
        std::string name;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texcoords;
        std::vector<glm::vec3> normals;
        std::vector<uint32_t> indices;
        Bounds bounds;

        bool hasTexcoords() const
        {
            return texcoords.size() == positions.size();
        }

        bool hasNormals() const
        {
            return normals.size() == positions.size();
        }

        bool isValid() const
        {
            return !positions.empty() && !indices.empty() && bounds.isValid();
        }
    };
}
