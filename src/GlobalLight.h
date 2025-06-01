//
// Created by andre on 1/06/2025.
//

#pragma once

struct LightUBO {
    glm::vec4 position{};
    glm::vec4 diffuse{};
};

struct GlobalLighting {
    LightUBO lights[10];
    int numLights = 0;
};