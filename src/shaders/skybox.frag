#version 450

layout(binding = 1) uniform samplerCube skyboxSampler;

layout(location = 0) in vec3 texCoord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(skyboxSampler, texCoord);
} 