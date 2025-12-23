#version 450

// 对应 C++ 里的 UniformBufferObject 结构体
// binding = 0 对应 createDescriptorSetLayout 里的 binding = 0
layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;  // C++ 第一个是 projection
    mat4 view;  // C++ 第二个是 view
    mat4 model; // C++ 第三个是 model
} ubo;

// 对应 C++ 里的 Vertex 结构体
layout(location = 0) in vec2 inPosition; // layout(location = 0)
layout(location = 1) in vec3 inColor;    // layout(location = 1)

// 输出到片段着色器
layout(location = 0) out vec3 fragColor;

void main() {
    // 矩阵乘法顺序：投影 * 视图 * 模型 * 顶点
    // 注意：inPosition 是 vec2，我们要补全成 vec4(x, y, 0.0, 1.0)
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);

    // 把颜色透传给片段着色器
    fragColor = inColor;
}