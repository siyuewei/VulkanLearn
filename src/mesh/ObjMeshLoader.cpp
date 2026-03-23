#include "ObjMeshLoader.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Mesh
{
    namespace
    {
        struct IndexKey
        {
            int pos = -1;
            int texcoord = -1;
            int normal = -1;

            bool operator==(const IndexKey &other) const
            {
                return pos == other.pos && texcoord == other.texcoord && normal == other.normal;
            }
        };

        struct IndexKeyHash
        {
            size_t operator()(const IndexKey &k) const
            {
                const size_t p = static_cast<size_t>(k.pos + 1);
                const size_t t = static_cast<size_t>(k.texcoord + 1);
                const size_t n = static_cast<size_t>(k.normal + 1);
                return (p * 73856093u) ^ (t * 83492791u) ^ (n * 19349663u);
            }
        };

        bool parseFaceToken(const std::string &token, int &posIndex, int &texcoordIndex, int &normalIndex)
        {
            posIndex = -1;
            texcoordIndex = -1;
            normalIndex = -1;

            const size_t firstSlash = token.find('/');
            if (firstSlash == std::string::npos)
            {
                posIndex = std::stoi(token);
                return true;
            }

            const std::string posPart = token.substr(0, firstSlash);
            if (!posPart.empty())
            {
                posIndex = std::stoi(posPart);
            }

            const size_t secondSlash = token.find('/', firstSlash + 1);
            if (secondSlash == std::string::npos)
            {
                const std::string texcoordPart = token.substr(firstSlash + 1);
                if (!texcoordPart.empty())
                {
                    texcoordIndex = std::stoi(texcoordPart);
                }
                return posIndex != -1;
            }

            const std::string texcoordPart = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
            if (!texcoordPart.empty())
            {
                texcoordIndex = std::stoi(texcoordPart);
            }

            if (secondSlash + 1 < token.size())
            {
                normalIndex = std::stoi(token.substr(secondSlash + 1));
            }

            return posIndex != -1;
        }

        int resolveObjIndex(int idx, int count)
        {
            if (idx > 0)
            {
                return idx - 1;
            }
            if (idx < 0)
            {
                return count + idx;
            }
            return -1;
        }
    }

    bool ObjMeshLoader::loadFromFile(const std::string &path, MeshAsset &outMesh, std::string &outError)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            outError = "无法打开 OBJ 文件: " + path;
            return false;
        }

        std::vector<glm::vec3> rawPositions;
        std::vector<glm::vec2> rawTexcoords;
        std::vector<glm::vec3> rawNormals;
        std::unordered_map<IndexKey, uint32_t, IndexKeyHash> uniqueVertexMap;
        bool needsNormalRebuild = false;

        MeshAsset mesh;
        mesh.name = path;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            std::istringstream iss(line);
            std::string tag;
            iss >> tag;

            if (tag == "v")
            {
                glm::vec3 p{};
                iss >> p.x >> p.y >> p.z;
                rawPositions.push_back(p);
            }
            else if (tag == "vn")
            {
                glm::vec3 n{};
                iss >> n.x >> n.y >> n.z;
                rawNormals.push_back(glm::normalize(n));
            }
            else if (tag == "vt")
            {
                glm::vec2 uv{};
                iss >> uv.x >> uv.y;
                // OBJ UV origin is lower-left; flip V to match Vulkan texture convention.
                uv.y = 1.0f - uv.y;
                rawTexcoords.push_back(uv);
            }
            else if (tag == "f")
            {
                std::vector<uint32_t> faceIndices;
                std::string token;
                while (iss >> token)
                {
                    int posObj = -1;
                    int texcoordObj = -1;
                    int normalObj = -1;
                    if (!parseFaceToken(token, posObj, texcoordObj, normalObj))
                    {
                        outError = "解析 face 失败: " + token;
                        return false;
                    }

                    const int posIndex = resolveObjIndex(posObj, static_cast<int>(rawPositions.size()));
                    const int texcoordIndex = resolveObjIndex(texcoordObj, static_cast<int>(rawTexcoords.size()));
                    const int normalIndex = resolveObjIndex(normalObj, static_cast<int>(rawNormals.size()));

                    if (posIndex < 0 || posIndex >= static_cast<int>(rawPositions.size()))
                    {
                        outError = "OBJ 顶点索引越界: " + token;
                        return false;
                    }
                    if (texcoordObj != -1 && (texcoordIndex < 0 || texcoordIndex >= static_cast<int>(rawTexcoords.size())))
                    {
                        outError = "OBJ UV 索引越界: " + token;
                        return false;
                    }
                    if (normalObj != -1 && (normalIndex < 0 || normalIndex >= static_cast<int>(rawNormals.size())))
                    {
                        outError = "OBJ 法线索引越界: " + token;
                        return false;
                    }

                    const IndexKey key{
                        posIndex,
                        texcoordObj == -1 ? -1 : texcoordIndex,
                        normalObj == -1 ? -1 : normalIndex};
                    auto it = uniqueVertexMap.find(key);
                    if (it == uniqueVertexMap.end())
                    {
                        const uint32_t newIndex = static_cast<uint32_t>(mesh.positions.size());
                        uniqueVertexMap.emplace(key, newIndex);
                        mesh.positions.push_back(rawPositions[posIndex]);
                        mesh.bounds.expand(rawPositions[posIndex]);

                        if (texcoordObj != -1)
                        {
                            mesh.texcoords.push_back(rawTexcoords[texcoordIndex]);
                        }
                        else
                        {
                            mesh.texcoords.push_back(glm::vec2(0.0f));
                        }

                        if (normalObj != -1)
                        {
                            mesh.normals.push_back(rawNormals[normalIndex]);
                        }
                        else
                        {
                            mesh.normals.push_back(glm::vec3(0.0f));
                            needsNormalRebuild = true;
                        }

                        faceIndices.push_back(newIndex);
                    }
                    else
                    {
                        faceIndices.push_back(it->second);
                    }
                }

                if (faceIndices.size() < 3)
                {
                    continue;
                }

                // Fan triangulation for n-gons.
                for (size_t i = 1; i + 1 < faceIndices.size(); i++)
                {
                    mesh.indices.push_back(faceIndices[0]);
                    mesh.indices.push_back(faceIndices[i]);
                    mesh.indices.push_back(faceIndices[i + 1]);
                }
            }
        }

        if (needsNormalRebuild)
        {
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                const uint32_t ia = mesh.indices[i];
                const uint32_t ib = mesh.indices[i + 1];
                const uint32_t ic = mesh.indices[i + 2];

                const glm::vec3 &a = mesh.positions[ia];
                const glm::vec3 &b = mesh.positions[ib];
                const glm::vec3 &c = mesh.positions[ic];

                const glm::vec3 faceNormal = glm::cross(b - a, c - a);

                if (glm::length(faceNormal) > 1e-6f)
                {
                    if (glm::length(mesh.normals[ia]) <= 1e-6f)
                    {
                        mesh.normals[ia] += faceNormal;
                    }
                    if (glm::length(mesh.normals[ib]) <= 1e-6f)
                    {
                        mesh.normals[ib] += faceNormal;
                    }
                    if (glm::length(mesh.normals[ic]) <= 1e-6f)
                    {
                        mesh.normals[ic] += faceNormal;
                    }
                }
            }

            for (glm::vec3 &n : mesh.normals)
            {
                if (glm::length(n) <= 1e-6f)
                {
                    n = glm::vec3(0.0f, 0.0f, 1.0f);
                }
                else
                {
                    n = glm::normalize(n);
                }
            }
        }

        if (!mesh.isValid())
        {
            outError = "OBJ 没有有效几何数据: " + path;
            return false;
        }

        outMesh = std::move(mesh);
        outError.clear();
        return true;
    }
}
