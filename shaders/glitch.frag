#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

// Time-based animation
float time = float(uint(gl_FragCoord.x + gl_FragCoord.y) % 6311) / 6311.0;

// Random function
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// RGB split with random offset
vec3 rgbSplitGlitch(sampler2D tex, vec2 uv) {
    float offset = (rand(vec2(time, uv.y)) - 0.5) * 0.05;
    
    vec3 col;
    col.r = texture(tex, vec2(uv.x + offset, uv.y)).r;
    col.g = texture(tex, vec2(uv.x, uv.y)).g;
    col.b = texture(tex, vec2(uv.x - offset, uv.y)).b;
    return col;
}

// Block glitch effect
vec2 blockGlitch(vec2 uv) {
    float block = floor(uv.y * 10.0);
    float noise = rand(vec2(block, time));
    
    if (noise > 0.95) {
        uv.x += (noise - 0.95) * 2.0;
    }
    
    return uv;
}

// Scanline effect with distortion
float scanline(vec2 uv) {
    float wave = sin(uv.y * 400.0 + time * 10.0) * 0.5 + 0.5;
    float scanline = step(0.5, wave) * 0.15;
    return 1.0 - scanline;
}

void main() {
    // Apply block glitch to UV coordinates
    vec2 uv = blockGlitch(fragTexCoord);
    
    // Get color with RGB split
    vec3 color = rgbSplitGlitch(screenTexture, uv);
    
    // Add scanlines
    color *= scanline(uv);
    
    // Random color shift
    if (rand(vec2(time, uv.x)) > 0.99) {
        color = vec3(1.0) - color;
    }
    
    // Add vertical noise bars
    float noise_x = floor(uv.x * 20.0);
    if (rand(vec2(noise_x, time)) > 0.98) {
        color *= 0.5;
    }
    
    // Add random bright pixels
    if (rand(uv + time) > 0.995) {
        color = vec3(1.0);
    }
    
    // Add wave distortion
    float wave = sin(uv.y * 10.0 + time * 5.0) * 0.1;
    color *= (1.0 + wave);
    
    outColor = vec4(color, 1.0);
} 