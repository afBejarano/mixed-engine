//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 input_uv;

layout (location = 0) out vec2 vertex_uv;
layout (location = 1) out vec3 worldNormal;
layout (location = 2) out vec3 worldPos;

layout (push_constant) uniform Model {
    mat4 transformation;
} model;

void main() {
    vec4 worldPosition = model.transformation * vec4(input_position, 1.0);
    gl_Position = camera.projection * camera.view * worldPosition;
    
    // Transform normal to world space
    mat3 normalMatrix = transpose(inverse(mat3(model.transformation)));
    worldNormal = normalize(normalMatrix * normal);
    
    // Pass world position and UV coordinates
    worldPos = worldPosition.xyz;
    vertex_uv = input_uv;
}