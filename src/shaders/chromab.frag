#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D screenTexture;

float aberration = 0.005;
vec3 col;

// RGB split for chromatic aberration
vec3 rgbSplit(sampler2D tex, vec2 uv) {
    col.r = texture(tex, vec2(uv.x + aberration, uv.y)).r;
    col.g = texture(tex, uv).g;
    col.b = texture(tex, vec2(uv.x - aberration, uv.y)).b;
    return col;
}

void main() {
    vec3 color = rgbSplit(screenTexture, fragTexCoord);
    outColor = vec4(color, 1.0);
}