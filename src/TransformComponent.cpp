//
// Created by Yibuz Pokopodrozo on 2025-05-19.
//

#include "TransformComponent.h"

TransformComponent::TransformComponent(Component* parent_):Component(parent_) {
    pos = glm::vec3(0.0f, 0.0f, 0.0f);
    orientation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

TransformComponent::TransformComponent(Component* parent_, glm::vec3 pos_, glm::quat orientation_, glm::vec3 scale_):
    Component{ parent_ }, pos{ pos_ }, orientation{ orientation_ }, scale{ scale_ } {
}

TransformComponent::~TransformComponent() = default;

bool TransformComponent::OnCreate() {
    if (isCreated == true) return true;
    isCreated = true;
    return true;
}

void TransformComponent::OnDestroy() {}

void TransformComponent::Update(const float deltaTime) {
    std::cout << "Hello from Update " << deltaTime << '\n';
}

void TransformComponent::Render()const {}

glm::mat4 TransformComponent::GetTransformMatrix() const {
    auto transform = glm::mat4(1.0f);
    transform = glm::scale(transform, scale);
    transform = transform * glm::mat4_cast(orientation);
    transform = glm::translate(transform, pos);
    return transform;
}