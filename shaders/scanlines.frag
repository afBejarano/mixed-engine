#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

float scanlineIntensity = 0.4;

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    float scanline = sin(fragTexCoord.y * 400.0) * scanlineIntensity + (1.0 - scanlineIntensity);
    color.rgb *= scanline;

    outColor = color;
}