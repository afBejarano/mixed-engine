//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once

#include <components/Actor.h>
#include <render/Renderer.h>

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

    GlobalLighting* global_lighting_ = nullptr;
private:
    Renderer* renderer;
    std::vector<Component*> components_;
};
