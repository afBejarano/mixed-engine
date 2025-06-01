//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

layout(set = 0, binding = 0) uniform UniformTransformations {
    mat4 view;
    mat4 projection;
} camera;

struct Light {
    vec4 position;  // w=0 for directional, w=1 for point light
    vec4 color;     // rgb = color, a = intensity
    vec4 params;    // x = constant, y = linear, z = quadratic attenuation
};

#define MAX_LIGHTS 4

layout(set = 0, binding = 1) uniform LightBuffer {
    vec4 viewPos;        // Camera position in world space
    uint numLights;      // Number of active lights
    Light lights[MAX_LIGHTS];
} lightBuffer;