//
// Created by Yibuz Pokopodrozo on 2025-05-10.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

//#include "ModelMatrixPushConstant.h"
#include <Components/Component.h>

/*struct BufferMemory {
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
};*/

class Actor : public Component {
    std::vector<std::shared_ptr<Component> > components;

protected:
    glm::mat4 modelMatrix;

public:
    Actor(const Actor &) = delete;
    Actor(Actor &&) = delete;
    Actor &operator=(const Actor &) = delete;
    Actor &operator=(Actor &&) = delete;

    Actor(Component *parent_): Component(parent_) {}

    Actor(): Component(nullptr) {}

    /*VkImage textureImage;
    VkImageView textureImageView;
    VkSampler imageSampler;
    IndexedBufferMemory modelBufferedMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;*/

    bool OnCreate() override;
    ~Actor() override;
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

    glm::mat4 GetModelMatrix();

    void ListComponents();
    void RemoveAllComponents();
};
