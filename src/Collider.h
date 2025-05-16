//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once

enum ColliderType{
    sphere, box
};

class Collider{
private:
    ColliderType type;
    float colliderRadius;
    glm::vec3 sPosition;
public:
    glm::vec3 maxCorner;
    glm::vec3 minCorner;
    bool active = true;
    Collider(const glm::vec3 nsPosition, const float nColliderRadius) : colliderRadius(nColliderRadius), sPosition(nsPosition) {
        type = ColliderType::sphere;
    }

    Collider(const glm::vec3 nMaxCorner, const glm::vec3 nMinCorner) : maxCorner(nMaxCorner), minCorner(nMinCorner) {
        type = ColliderType::box;
    }

    void setActive(const bool active_){
        active = active_;
    }

    void setPosition(const glm::vec3 nPosition){
        sPosition = nPosition;
    }

    static bool boxBox(const Collider c1, const Collider c2 ) {
        return (
            c1.minCorner.x <= c2.maxCorner.x &&
            c1.maxCorner.x >= c2.minCorner.x &&
            c1.minCorner.y <= c2.maxCorner.y &&
            c1.maxCorner.y >= c2.minCorner.y &&
            c1.minCorner.z <= c2.maxCorner.z &&
            c1.maxCorner.z >= c2.minCorner.z
        );
    }

    static bool sphereSphere (const Collider c1, const Collider c2 ) {
        return glm::distance(c1.sPosition, c2.sPosition) < c1.colliderRadius + c2.colliderRadius;
    }

    static bool sphereBox (const Collider c1, const Collider c2 ) {

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

    [[nodiscard]] bool isColliding(const Collider c1) const {
        if (!active || !c1.active)
            return false;
        if (type == ColliderType::box) {
            if(c1.type == ColliderType::sphere) {
                return sphereBox(c1, *this);
            }
            else {
                return boxBox(c1, *this);
            }
        }
        else{
            if(c1.type == ColliderType::sphere){
                return sphereSphere(*this, c1);
            }
            else {
                return sphereBox(*this, c1);
            }
        }
    }
};