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
    global_lighting_->lights[0] = LightUBO{{7.0, -3.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}};  // Red light
    global_lighting_->lights[1] = LightUBO{{0.0, -3.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}};  // Green light
    global_lighting_->lights[2] = LightUBO{{-7.0, -3.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}};  // Blue light
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
        
        // Debug output for light data
        for (int i = 0; i < global_lighting_->numLights; i++) {
            spdlog::info("Light {}: Position ({}, {}, {}, {}), Color ({}, {}, {}, {})",
                i,
                global_lighting_->lights[i].position.x,
                global_lighting_->lights[i].position.y,
                global_lighting_->lights[i].position.z,
                global_lighting_->lights[i].position.w,
                global_lighting_->lights[i].diffuse.x,
                global_lighting_->lights[i].diffuse.y,
                global_lighting_->lights[i].diffuse.z,
                global_lighting_->lights[i].diffuse.w
            );
        }
        
        vRenderer->SetLightsUBO(global_lighting_);
    }
    for (Component *component: components_) {
        component->Render();
    }
}

void Scene::AddActor(Actor *actor) {
    components_.push_back(actor);
}
