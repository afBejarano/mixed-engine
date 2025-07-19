//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

layout(set = 0, binding = 0) uniform UniformTransformations {
    mat4 view;
    mat4 projection;
    vec3 viewPos;
} camera;