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
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightDir = vec3(normalize(light.position - vec4(FragPos,1)));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, reflectDir), 0.0), 5.0);
    float distance = length(vec3(light.position) - FragPos);
    float attenuation = 1.0 / distance;
    vec3 ambient  = ubo.ambient  * vec3(texture(texture_sampler, vertex_uv));
    vec3 diffuse  = vec3(light.diffuse) * diff * vec3(texture(texture_sampler, vertex_uv));
    vec3 specular = ubo.specular * spec * attenuation;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return vec4(ambient + diffuse + specular, 1.0);
}

void main() {
    vec4 sum = vec4(0.0,0.0,0.0,0.0);
    for(int i = 0; i < glights.numLights; i++){
        sum += CalcPointLight(glights.lights[i])/glights.numLights;
    }
    out_color = sum;
}