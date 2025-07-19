#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

// Real time uniform
layout(push_constant) uniform PushConstants {
    float time;  // Real time in seconds
} push;

vec4[] colors = {vec4(1, 0, 0, 0.75), vec4(0, 1, 0, 0.75), vec4(0, 0, 1, 0.75), vec4(0, 0, 0, 0.5)};
float[] factors = {0.97,0.97,0.97,0.85};

void main() {
    float x = floor(gl_FragCoord.x / 3);
    float y = floor(gl_FragCoord.y / 4);

    vec4 color = texture(screenTexture, fragTexCoord.xy);

    color = mix(color * 1.5, color, 1 - (1 / (mod(gl_FragCoord.y * x - (push.time * 2),256))));

    int i = int(mod(x,4));
    color = mix(colors[i], color, factors[i]);

    color = mix(color * 0.1, color, abs(sin(y + push.time)));

    outColor = color;
} 