//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout (location = 0) in vec2 vertex_uv;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 FragPos;
layout (location = 3) in vec3 viewPos;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D texture_sampler;
layout(set = 1, binding = 0) uniform UniformBufferObject {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} ubo;

struct LightUBO{
    vec4 position;
    vec4 diffuse;
};

layout(set = 3, binding = 0) uniform GlobalLightingUBO {
    LightUBO lights[10];
    int numLights;
} glights;

vec4 CalcPointLight(LightUBO light)
{
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(vec3(light.position) - FragPos);
    
    // Simple diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Get base color from texture
    vec3 texColor = vec3(texture(texture_sampler, vertex_uv));
    
    // Combine light color with texture and diffuse factor
    vec3 result = vec3(light.diffuse) * texColor * diff;
    
    return vec4(result, 1.0);
}

void main() {
    // Start with base ambient lighting
    vec3 texColor = vec3(texture(texture_sampler, vertex_uv));
    vec4 result = vec4(texColor * 0.2, 1.0);  // 0.2 is ambient intensity
    
    // Add contribution from each light
    for(int i = 0; i < glights.numLights; i++) {
        result += CalcPointLight(glights.lights[i]);
    }
    
    // Ensure we don't exceed maximum brightness
    out_color = clamp(result, 0.0, 1.0);
}