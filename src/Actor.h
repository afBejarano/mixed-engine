#pragma once

//#include "ModelMatrixPushConstant.h"
#include "Collider.h"
#include "precomp.h"

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

class Actor{
public:
    float thetaRadianRotation;
    float gammaRadianRotation;
    glm::vec3 position;
    //ModelMatrixPushConst modelMatrixPushConst;
    glm::vec3 scale;
    std::string model;
    std::string texture;
    Collider* collider;
    Actor(float nThetaRadianRotation, float nGammaRadianRotation, glm::vec3 nPosition, glm::vec3 nScale, std::string nModel, std::string nTexture, Collider* nCollider):
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
};