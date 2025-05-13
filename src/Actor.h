//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once

//#include "ModelMatrixPushConstant.h"
#include "Collider.h"
#include "precomp.h"
#include "Component.h"

struct BufferMemory {
    VkBuffer bufferID;
    VkDeviceMemory bufferMemoryID;
};

struct IndexedBufferMemory {
    VkBuffer vertBufferID;
    VkDeviceMemory vertBufferMemoryID;
    VkDeviceSize vertBufferSize;

    VkBuffer indexBufferID;
    VkDeviceMemory indexBufferMemoryID;
    VkDeviceSize indexBufferSize;
};

class Actor : public Component{

std::vector<Component*> components;

public:

    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator= (const Actor&) = delete;
    Actor& operator=(Actor&&) = delete;

    float thetaRadianRotation;
    float gammaRadianRotation;
    glm::vec3 position;
    //ModelMatrixPushConst modelMatrixPushConst;
    glm::vec3 scale;
    std::string model;
    std::string texture;
    Collider* collider;
    Actor(float nThetaRadianRotation, float nGammaRadianRotation, glm::vec3 nPosition, glm::vec3 nScale, std::string nModel, std::string nTexture, Collider* nCollider, Component* parent_):
    thetaRadianRotation(nThetaRadianRotation), gammaRadianRotation(nGammaRadianRotation), position(nPosition), scale(nScale), model(std::move(nModel)),
        texture(std::move(nTexture)), collider(nCollider) {}
    Actor(float nThetaRadianRotation, float nGammaRadianRotation, glm::vec3 nPosition, glm::vec3 nScale, std::string nModel, std::string nTexture):
    thetaRadianRotation(nThetaRadianRotation), gammaRadianRotation(nGammaRadianRotation), position(nPosition), scale(nScale), model(std::move(nModel)),
        texture(std::move(nTexture)) {}
    void move(glm::vec3 newPos) {
        position = newPos;
        if(collider)
            collider->setPosition(newPos);
    }
    void setTheta(float tTheta){
        gammaRadianRotation = tTheta;
    }
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler imageSampler;
    IndexedBufferMemory modelBufferedMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime) override;
    void Render() const override;

    template<typename ComponentTemplate, typename ... Args>
    void AddComponent(Args&& ... args_) {
        auto* componentObject = new ComponentTemplate(std::forward<Args>(args_)...);
        components.push_back(componentObject);

    }

    template<typename ComponentTemplate>
    ComponentTemplate* GetComponent() {
        for (auto component : components) {
            if (dynamic_cast<ComponentTemplate*>(component) != nullptr) {
                return dynamic_cast<ComponentTemplate*>(component);
            }
        }
        return nullptr;
    }

    template<typename ComponentTemplate>
    void RemoveComponent() {
        for (size_t i = 0; i < components.size(); i++) {
            if (dynamic_cast<ComponentTemplate*>(components[i]) != nullptr) {
                components[i]->OnDestroy();
                delete components[i];
                components.erase(components.begin() + i);
                break;
            }
        }
    }

    void ListComponents() const;
    void RemoveAllComponents();
};