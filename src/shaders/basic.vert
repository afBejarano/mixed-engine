//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec2 uv_in;

layout(location = 0) out vec2 uv_out;

void main() {
    gl_Position = vec4(input_position, 1.0);
    uv_out = uv_in;
}