//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 input_uv;

layout (location = 0) out vec2 vertex_uv;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec3 FragPos;
layout (location = 3) out vec3 viewPos;

layout (push_constant) uniform Model {
    mat4 transformation;
} model;

void main() {
    vertex_uv = input_uv;
    oNormal = normal.xyz;
    vec4 vVertex1 = vec4(input_position.x, input_position.y, input_position.z, 1);
    FragPos = vec3(model.transformation * vVertex1);

    viewPos = camera.viewPos;

    gl_Position =  camera.projection * camera.view * model.transformation * vVertex1;
}
