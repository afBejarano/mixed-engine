#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

const float brightness = 2.0;

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    color *= brightness;
    outColor = color;
}