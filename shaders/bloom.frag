#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

const float threshold = 0.8; // Brightness threshold
const float intensity = 2.0; // Bloom strength
const float exposure = 1.2; // Overall brightness
const int count = 64; // Number of samples
const float scale = 0.002; // Scale of the blur samples

float getLuminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 getBrightAreas(vec3 color) {
    float brightness = getLuminance(color);
    return color * smoothstep(threshold - 0.1, threshold + 0.1, brightness);
}

vec3 getBlur(vec2 uv) {
    vec3 blur = vec3(0.0);
    float total_weight = 0.0;

    for (int i = 0; i < count; i++) {
        float angle = float(i) * (2.0 * 3.14159 / float(count));
        float distance = 1.0 - (float(i) / float(count));

        vec2 offset = vec2(cos(angle) * scale * distance, sin(angle) * scale * distance);

        vec3 sample_color = getBrightAreas(texture(screenTexture, uv + offset).rgb);
        float weight = 1.0 / (1.0 + float(i));

        blur += sample_color * weight;
        total_weight += weight;
    }

    return blur / total_weight;
}

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 originalColor = texture(screenTexture, fragTexCoord).rgb;
    vec3 bloomColor = getBlur(fragTexCoord);
    vec3 finalColor = originalColor + bloomColor * intensity;
    finalColor *= exposure;
    finalColor = aces(finalColor);
    outColor = vec4(finalColor, 1.0);
}