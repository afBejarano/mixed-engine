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
        float radians = glm::radians(180.0f); // Convert degrees to radians
        float halfAngle = radians / 2.0f;
        float radians2 = glm::radians(90.0f); // Convert degrees to radians
        float halfAngle2 = radians2 / 2.0f;

        glm::quat rotation1 = glm::quat(cos(halfAngle), sin(halfAngle), 0.0f, 0.0f);
        glm::quat rotation2 = glm::quat(cos(halfAngle2), sin(halfAngle2), 0.0f, 0.0f);

        VulkanRenderer* vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
        Actor* actor = new Actor(nullptr);
        actor->AddComponent<ObjectComponent>("./assets/Spaceship/Intergalactic_Spaceship-(Wavefront).obj", "./assets/Spaceship/", actor, vRenderer);
        actor->AddComponent<TransformComponent, Component*, glm::vec3, glm::quat, glm::vec3>(actor, {-5.0f, 0.0f, 0.0f}, std::move(rotation1), {1.0f, 1.0f, 1.0f});

        components_.push_back(actor);

        Actor* actor2 = new Actor(nullptr);
        actor2->AddComponent<ObjectComponent>("./assets/Skull/12140_Skull_v3_L2.obj", "./assets/Skull/", actor2, vRenderer);
        actor2->AddComponent<TransformComponent, Component*, glm::vec3, glm::quat, glm::vec3>(actor2, {50.0f, 0.0f, 0.0f}, std::move(rotation2), {0.1f, 0.1f, 0.1f});

        components_.push_back(actor2);

        glm::vec2 size = vRenderer->GetWindowSize();
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -1.0f, -10.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspectiveZO(glm::radians(60.0f), size.x / size.y, 0.1f, 100.0f);
        vRenderer->SetViewProjection(view, proj);
    }
    return true;
}

void Scene::Render() {
    for (Component* component : components_) {
        component->Render();
    }
}
