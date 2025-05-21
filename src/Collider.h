//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once
#include <precomp.h>
#include <Components/Component.h>

enum ColliderType {
    sphere, box
};

class Collider final : public Component {
    std::vector<std::shared_ptr<Component>> components;

public:
    glm::vec3 maxCorner;
    glm::vec3 minCorner;
    bool active = true;

    Collider(const glm::vec3 nsPosition, const float nColliderRadius, Component* parent_) : Component(parent_),
        maxCorner({}), minCorner({}), colliderRadius(nColliderRadius), sPosition(nsPosition) {
        type = sphere;
    }

    Collider(const glm::vec3 nMaxCorner, const glm::vec3 nMinCorner, Component* parent_) : Component(parent_),
        maxCorner(nMaxCorner), minCorner(nMinCorner), colliderRadius({}), sPosition({}) {
        type = box;
    }

    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime) override;
    void Render() const override;

    template<typename ComponentTemplate, typename... Args>
    void AddComponent(Args &&... args_) {
        auto *componentObject = new ComponentTemplate(std::forward<Args>(args_)...);
        components.push_back(componentObject);
    }

    template<typename ComponentTemplate>
    std::shared_ptr<ComponentTemplate> GetComponent() {
        for (std::shared_ptr component: components) {
            if (std::shared_ptr<ComponentTemplate> casted_component = std::dynamic_pointer_cast<
                ComponentTemplate>(component)) {
                return casted_component;
                }
        }
        return nullptr;
    }

    template<typename ComponentTemplate>
    void RemoveComponent() {
        for (size_t i = 0; i < components.size(); i++) {
            if (dynamic_cast<ComponentTemplate *>(components[i]) != nullptr) {
                components[i]->OnDestroy();
                delete components[i];
                components.erase(components.begin() + i);
                break;
            }
        }
    }

    void ListComponents() {
        std::cout << typeid(*this).name() << " contains the following components:\n";
        for (const auto& component : components) {
            std::cout << typeid(*component).name() << std::endl;
        }
        std::cout << '\n';
    }

    void RemoveAllComponents() {
        components.clear();
    }

    void setActive(const bool active_) {
        active = active_;
    }

    void setPosition(const glm::vec3 nPosition) {
        sPosition = nPosition;
    }

    static bool boxBox(const Collider& c1, const Collider& c2) {
        return (
            c1.minCorner.x <= c2.maxCorner.x &&
            c1.maxCorner.x >= c2.minCorner.x &&
            c1.minCorner.y <= c2.maxCorner.y &&
            c1.maxCorner.y >= c2.minCorner.y &&
            c1.minCorner.z <= c2.maxCorner.z &&
            c1.maxCorner.z >= c2.minCorner.z
        );
    }

    static bool sphereSphere(const Collider& c1, const Collider& c2) {
        return glm::distance(c1.sPosition, c2.sPosition) < c1.colliderRadius + c2.colliderRadius;
    }

    static bool sphereBox(const Collider& c1, const Collider& c2) {
        const float x = std::fmax(c2.minCorner.x, std::fmin(c1.sPosition.x, c2.maxCorner.x));
        const float y = std::fmax(c2.minCorner.y, std::fmin(c1.sPosition.y, c2.maxCorner.y));
        const float z = std::fmax(c2.minCorner.z, std::fmin(c1.sPosition.z, c2.maxCorner.z));

        const float distance = sqrtf(
            (x - c1.sPosition.x) * (x - c1.sPosition.x) +
            (y - c1.sPosition.y) * (y - c1.sPosition.y) +
            (z - c1.sPosition.z) * (z - c1.sPosition.z)
        );

        return distance < c1.colliderRadius;
    }

    [[nodiscard]] bool isColliding(const Collider& c1) const {
        if (!active || !c1.active)
            return false;

        if (type == box) {
            if (c1.type == sphere) {
                return sphereBox(c1, *this);
            } else {
                return boxBox(c1, *this);
            }
        }
        if (c1.type == sphere) {
            return sphereSphere(*this, c1);
        } else {
            return sphereBox(*this, c1);
        }
    }

private:
    ColliderType type;
    float colliderRadius;
    glm::vec3 sPosition;
};
