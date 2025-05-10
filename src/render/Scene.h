//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once
#include <string>

#include "Renderer.h"

class Scene {
public:
    Scene(Renderer* renderer_);
    ~Scene();

    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;

    bool OnCreate();
    void OnDestroy();
    void HandleEvents();

private:
    Renderer* renderer;
};
