//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once
#include <string>

#include "Renderer.h"
#include "components/Actor.h"

class Scene {
public:
    Scene(Renderer* renderer_);
    ~Scene();

    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;

    bool OnCreate();
    void OnDestroy();
    void HandleEvents();
    void Render();

    void AddActor(Actor* actor);

private:
    Renderer* renderer;
    std::vector<Component*> components_;
};
