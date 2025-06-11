#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

// Parameters for CRT effect
const float scanlineIntensity = 0.3;
const float curvature = 4.0;
const float vignetteStrength = 0.5;
const float brightness = 2.0;
const float aberration = 0.005;

vec3 col;

// RGB split for chromatic aberration
vec3 rgbSplit(sampler2D tex, vec2 uv) {
    col.r = texture(tex, vec2(uv.x + aberration, uv.y)).r;
    col.g = texture(tex, uv).g;
    col.b = texture(tex, vec2(uv.x - aberration, uv.y)).b;
    return col;
}

// Vignette effect
float vignette(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    return 1.0 - dot(uv, uv) * vignetteStrength;
}

void main() {
    // Apply RGB split
    vec3 color = rgbSplit(screenTexture, fragTexCoord);

    // Apply scanlines
    float scanline = sin(fragTexCoord.y * 400) * scanlineIntensity + (1 - scanlineIntensity);
    color.rgb *= scanline;

    // Apply vignette
    color *= vignette(fragTexCoord);

    // Increase brightness to compensate for effects
    color *= brightness;

    // Add slight bloom/glow
    vec3 blur = rgbSplit(screenTexture, fragTexCoord + vec2(0.002));
    color += blur * 0.1;

    outColor = vec4(color, 1.0);
} 