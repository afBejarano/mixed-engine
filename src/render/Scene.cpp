//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#include <render/Scene.h>

#include <components/Actor.h>

Scene::Scene(Renderer *renderer_) : renderer(renderer_) {
    OnCreate();
}

Scene::~Scene() {
    OnDestroy();
}

bool Scene::OnCreate() {
    return true;
}

void Scene::OnDestroy() {
    // First destroy all components
    for (auto *component: components_) {
        if (component) {
            component->OnDestroy();
        }
    }

    // Then delete and clear
    for (auto *component: components_) {
        delete component;
    }
    components_.clear();
}

void Scene::Render() {
    if (renderer->getRendererType() == RendererType::VULKAN) {
        VulkanRenderer *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
        vRenderer->SetLightsUBO(global_lighting_);
    }
    for (Component *component: components_) {
        component->Render();
    }
}

void Scene::AddActor(Actor *actor) {
    components_.push_back(actor);
}
