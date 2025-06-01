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
    global_lighting_ = new GlobalLighting{};
    global_lighting_->lights[0] = LightUBO{{5, 0, 0, 0}, {1.0, 0.0, 0.0, .5}};
    global_lighting_->lights[1] = LightUBO{{0, 0, 0, 0}, {0.0, 1.0, 0.0, .5}};
    global_lighting_->lights[2] = LightUBO{{-5, 0, 0, 0}, {0.0, 0.0, 1.0, .5}};
    global_lighting_->numLights = 3;
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
