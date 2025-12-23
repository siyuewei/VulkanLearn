#version 450

// 接收从顶点着色器传过来的颜色
layout(location = 0) in vec3 fragColor;

// 输出到屏幕 (location = 0 对应 SwapChain 的 attachment)
layout(location = 0) out vec4 outColor;

void main() {
    // 加上透明度 1.0
    outColor = vec4(fragColor, 1.0);
}