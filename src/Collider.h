//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once
#include <precomp.h>
#include <components/Component.h>

enum ColliderType {
    sphere = 1,
    box = 2
};

class Collider final : public Component {

public:
    glm::vec3 maxCorner;
    glm::vec3 minCorner;
    bool active = true;

    Collider(const glm::vec3 nsPosition, const float nColliderRadius, Component *parent_) : Component(parent_),
        maxCorner({}), minCorner({}), colliderRadius(nColliderRadius), sPosition(nsPosition) {
        type = sphere;
    }

    Collider(const glm::vec3 nMaxCorner, const glm::vec3 nMinCorner, Component *parent_) : Component(parent_),
        maxCorner(nMaxCorner), minCorner(nMinCorner), colliderRadius({}), sPosition({}) {
        type = box;
    }

    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime) override;
    void Render() const override;

    void setActive(const bool active_) {
        active = active_;
    }

    void setPosition(const glm::vec3 nPosition) {
        sPosition = nPosition;
    }

    static bool boxBox(const Collider &box_collider1, const Collider &box_collider2) {
        return (
            box_collider1.minCorner.x <= box_collider2.maxCorner.x &&
            box_collider1.maxCorner.x >= box_collider2.minCorner.x &&
            box_collider1.minCorner.y <= box_collider2.maxCorner.y &&
            box_collider1.maxCorner.y >= box_collider2.minCorner.y &&
            box_collider1.minCorner.z <= box_collider2.maxCorner.z &&
            box_collider1.maxCorner.z >= box_collider2.minCorner.z
        );
    }

    static bool sphereSphere(const Collider &sphere_collider1, const Collider &sphere_collider2) {
        return glm::distance(sphere_collider1.sPosition, sphere_collider2.sPosition) < sphere_collider1.colliderRadius +
               sphere_collider2.colliderRadius;
    }

    static bool sphereBox(const Collider &collider1, const Collider &collider2) {
        if (collider2.type == sphere && collider1.type == sphere || collider2.type == box && collider1.type == box)
            return false;
        if (collider2.type == sphere)
            return sphereBox(collider2, collider1);
        const float x = std::fmax(collider2.minCorner.x, std::fmin(collider1.sPosition.x, collider2.maxCorner.x));
        const float y = std::fmax(collider2.minCorner.y, std::fmin(collider1.sPosition.y, collider2.maxCorner.y));
        const float z = std::fmax(collider2.minCorner.z, std::fmin(collider1.sPosition.z, collider2.maxCorner.z));

        const float distance = sqrtf(
            (x - collider1.sPosition.x) * (x - collider1.sPosition.x) +
            (y - collider1.sPosition.y) * (y - collider1.sPosition.y) +
            (z - collider1.sPosition.z) * (z - collider1.sPosition.z)
        );

        return distance < collider1.colliderRadius;
    }

    [[nodiscard]] bool isColliding(const Collider &other_collider) const {
        if (!active || !other_collider.active)
            return false;
        switch (type + other_collider.type) {
            case sphere + sphere:
                return sphereSphere(*this, other_collider);
            case sphere + box:
                return sphereBox(*this, other_collider);
            case box + box:
                return boxBox(*this, other_collider);
            default:
                return false;
        }
    }

private:
    ColliderType type;
    float colliderRadius;
    glm::vec3 sPosition;
};
