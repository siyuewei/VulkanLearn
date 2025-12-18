#version 450

// layout(location = 0) 对应 C++ 代码里设置的属性位置 0
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main(){
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}