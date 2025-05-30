//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 in_tex_coord;
layout (location = 3) in vec4 color;
layout (location = 4) in vec4 tangent;

layout(set = 0, binding = 0) uniform UniformTransformations {
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec3 tex_coord;

void main() {
    tex_coord = input_position;
    gl_Position = ubo.projection * ubo.view * vec4(input_position, 1.0);
}