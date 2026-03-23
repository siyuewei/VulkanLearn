#pragma once

#include <string>

#include "MeshAsset.h"

namespace Mesh
{
    class ObjMeshLoader
    {
    public:
        static bool loadFromFile(const std::string &path, MeshAsset &outMesh, std::string &outError);
    };
}
