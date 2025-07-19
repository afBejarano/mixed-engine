#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

const float vignetteStrength = 0.5;

float vignette(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    return 1.0 - dot(uv, uv) * vignetteStrength;
}

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    color *= vignette(fragTexCoord);
    outColor = color;
}