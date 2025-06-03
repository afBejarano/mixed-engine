#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

float scanlineIntensity;
float pixelation;
float reductionIndex = 5.0;
float colorReduction = pow(2.0, reductionIndex);

void main() {
    // Pixelation
    vec2 pixels = vec2(pixelation);
    vec2 pixelated = floor(fragTexCoord * pixels) / pixels;
    vec4 color = texture(screenTexture, pixelated);

    // Color reduction
    color.rgb = floor(color.rgb * colorReduction) / colorReduction;

    // Scanlines
    float scanline = sin(fragTexCoord.y * 400.0) * scanlineIntensity +
    (1.0 - scanlineIntensity);
    color.rgb *= scanline;

    outColor = color;
}