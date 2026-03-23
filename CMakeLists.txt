cmake_minimum_required(VERSION 3.15)
project(VulkanLearn)

# ==========================================
# 1. 基础设置
# ==========================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 解决中文乱码 (MSVC)
if(MSVC)
add_compile_options("/utf-8" "/wd4819")
endif()

# ==========================================
# 2. 自动下载依赖 (FetchContent)
# ==========================================
include(FetchContent)

# --- GLFW ---
message(STATUS "Downloading GLFW...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

# 设置 GLFW 选项：我们不需要文档和测试，只想用它的库
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- GLM ---
message(STATUS "Downloading GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# ==========================================
# 3. 查找 Vulkan
# ==========================================
find_package(Vulkan REQUIRED)

# ==========================================
# 4. 构建项目
# ==========================================
set(VULKANLEARN_SOURCES
    main.cpp
    src/mesh/MeshAsset.h
    src/mesh/ObjMeshLoader.cpp
    src/mesh/ObjMeshLoader.h
    src/vulkan/VulkanLearnApplication.cpp
    src/vulkan/VulkanLearnApplication.h
    src/vulkan/VulkanCommands.cpp
    src/vulkan/VulkanCommands.h
    src/vulkan/VulkanCore.cpp
    src/vulkan/VulkanCore.h
    src/vulkan/VulkanPipeline.cpp
    src/vulkan/VulkanPipeline.h
    src/vulkan/VulkanResources.cpp
    src/vulkan/VulkanResources.h
    src/vulkan/VulkanSceneData.cpp
    src/vulkan/VulkanSceneData.h
    src/vulkan/VulkanSwapchain.cpp
    src/vulkan/VulkanSwapchain.h
    src/vulkan/VulkanUtils.cpp
    src/vulkan/VulkanUtils.h
)

add_executable(VulkanLearn ${VULKANLEARN_SOURCES})

target_include_directories(VulkanLearn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mesh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan
)

# 链接库：链接 Vulkan, GLFW 和 GLM
target_link_libraries(VulkanLearn PRIVATE 
    Vulkan::Vulkan
    glfw
    glm::glm
)

# 工程级配置：把路径/宏放到编译期，避免写死本机绝对路径
target_compile_definitions(VulkanLearn PRIVATE
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLFW_INCLUDE_VULKAN
    ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

if (MSVC)
    target_compile_options(VulkanLearn PRIVATE /W4)
else()
    target_compile_options(VulkanLearn PRIVATE -Wall -Wextra -Wpedantic)
endif()