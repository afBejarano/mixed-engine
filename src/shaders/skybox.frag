//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450

layout (location = 0) in vec3 tex_coord;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = vec4(tex_coord, 0.0);
}