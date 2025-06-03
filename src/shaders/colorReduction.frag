#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

float reductionIndex = 5.0;
float colorReduction = pow(2.0, reductionIndex);

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    color.rgb = floor(color.rgb * colorReduction) / colorReduction;
    outColor = color;
}