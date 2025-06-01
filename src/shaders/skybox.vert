#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 texCoord;

void main() {
    texCoord = inPosition;

    // Remove translation from view matrix to keep skybox centered on camera
    mat4 viewNoTranslation = mat4(mat3(ubo.view));
    vec4 pos = ubo.proj * viewNoTranslation * vec4(inPosition, 1.0);
    
    // Ensure skybox is always at maximum depth
    gl_Position = pos.xyww;
} 