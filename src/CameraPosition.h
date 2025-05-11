#pragma once
#include "precomp.h"
#include "Collider.h"

class CameraPosition {
public:
	glm::vec3& position;
	float theta;
	float gamma;
	Collider* collider;
	Collider* actionCollider;
    CameraPosition(glm::vec3& nPosition, float nTheta, float nGamma, Collider* nCollider, Collider* nActionCollider):
	position(nPosition), theta(nTheta), gamma(nGamma), collider(nCollider),actionCollider(nActionCollider){};
	void setPosition(glm::vec3 nPosition) const {
		position = nPosition;
		collider->setPosition(glm::vec3{-position.x, -position.y, -position.z});
		actionCollider->setPosition(glm::vec3{-position.x, -position.y, -position.z});
	}
    void setTheta(float nTheta){
        theta = nTheta;
    }
    void setGamma(float nGamma){
        gamma = nGamma;
    }
};