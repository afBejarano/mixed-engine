//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout(location = 0) in vec2 uv;

layout(binding=0, set=1) uniform sampler2D imageSampler;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(imageSampler, uv);
}