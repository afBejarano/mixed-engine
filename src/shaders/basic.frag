//
// Created by Yibuz Pokopodrozo on 2025-05-11.
//

#version 450
#include "common.glsl"

layout (location = 0) in vec2 vertex_uv;
layout (location = 1) in vec3 worldNormal;
layout (location = 2) in vec3 worldPos;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D texture_sampler;
layout(set = 1, binding = 0) uniform UniformBufferObject {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

vec3 calculateLight(Light light, vec3 normal, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir;
    float attenuation = 1.0;
    
    if (light.position.w == 0.0) {
        // Directional light
        lightDir = normalize(-light.position.xyz);
    } else {
        // Point light
        lightDir = light.position.xyz - worldPos;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);
        
        // Calculate attenuation
        attenuation = 1.0 / (light.params.x + 
                            light.params.y * distance +
                            light.params.z * distance * distance);
    }
    
    // Ambient
    vec3 ambient = material.ambient * light.color.rgb;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * baseColor * light.color.rgb;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * material.specular * light.color.rgb;
    
    // Combine with attenuation and intensity
    return (ambient + diffuse + specular) * attenuation * light.color.a;
}

void main() {
    vec3 normal = normalize(worldNormal);
    vec3 viewDir = normalize(lightBuffer.viewPos.xyz - worldPos);
    
    // Get base color from texture
    vec3 baseColor = texture(texture_sampler, vertex_uv).rgb * material.diffuse;
    
    // Calculate lighting from all lights
    vec3 result = vec3(0.0);
    for(uint i = 0; i < lightBuffer.numLights; i++) {
        result += calculateLight(lightBuffer.lights[i], normal, viewDir, baseColor);
    }
    
    // Add global ambient light
    result += baseColor * 0.1; // Small global ambient term
    
    // HDR tonemapping
    result = result / (result + vec3(1.0));
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    out_color = vec4(result, 1.0);
}