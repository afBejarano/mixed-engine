//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#include "Scene.h"

#include "components/Actor.h"

Scene::Scene(Renderer* renderer_) : renderer(renderer_) {
    OnCreate();
}

Scene::~Scene() {
}

bool Scene::OnCreate() {
    if (renderer->getRendererType() == RendererType::VULKAN) {
    }
    return true;
}

void Scene::Render() {
    for (Component* component : components_) {
        component->Render();
    }
}

void Scene::AddActor(Actor *actor) {
    components_.push_back(actor);
}
