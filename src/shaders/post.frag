#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D screenTexture;

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    
    // Convert to grayscale using luminance weights
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    outColor = vec4(gray, gray, gray, color.a);
} 