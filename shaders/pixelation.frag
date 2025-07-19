#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

float pixelation = 140.0;

void main() {
    vec2 pixels = vec2(pixelation);
    vec2 pixelated = floor(fragTexCoord * pixels) / pixels;
    vec4 color = texture(screenTexture, pixelated);
    outColor = color;
}