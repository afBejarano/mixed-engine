//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#include "Scene.h"

Scene::Scene(Renderer* renderer_) : renderer(renderer_) {
    OnCreate();
}

Scene::~Scene() {
}

bool Scene::OnCreate() {
    return true;
}
