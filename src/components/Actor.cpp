//
// Created by Yibuz Pokopodrozo on 2025-05-12.
//

#include <components/Actor.h>
#include <Debug.h>
#include <components/TransformComponent.h>
#include <glm/gtx/rotate_vector.hpp>

bool Actor::OnCreate() {
    if (isCreated) return true;
    Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
    for (const auto& component : components) {
        if (component->OnCreate() == false) {
            Debug::Error("Loading assets for Actor/Components: ", __FILE__, __LINE__);
            isCreated = false;
            return isCreated;
        }
    }
    isCreated = true;
    return isCreated;
}

Actor::~Actor() {
    Actor::OnDestroy();
}

void Actor::OnDestroy() {
    Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
    RemoveAllComponents();
    isCreated = false;
}

void Actor::Update(const float deltaTime) {
    std::cout << "Hello from Update\n";
}

void Actor::Render()const {
    for (Component* component : components) {
        component->Render();
    }
}

void Actor::RemoveAllComponents() {
    // First destroy all components
    for (auto* component : components) {
        if (component) {
            component->OnDestroy();
        }
    }
    
    // Then delete and clear
    for (auto* component : components) {
        delete component;
    }
    components.clear();
}

void Actor::ListComponents() {
    std::cout << typeid(*this).name() << " contains the following components:\n";
    for (const auto& component : components) {
        std::cout << typeid(*component).name() << std::endl;
    }
    std::cout << '\n';
}

glm::mat4 Actor::GetModelMatrix() {
    TransformComponent* transform = GetComponent<TransformComponent>();
    if (transform) {
        modelMatrix = transform->GetTransformMatrix();
    } else {
        modelMatrix = glm::mat4(1.0f);
    }
    if (parent) {
        modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix() * modelMatrix;
    }
    return modelMatrix;
}