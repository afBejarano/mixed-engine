//
// Created by andre on 2025-05-21.
//

#pragma once
#include <components/Component.h>
#include <render/VulkanRenderer.h>

struct Mesh {
    std::vector<int> indices;
    int materialId;
};

struct Material_UBO {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct material {
    Material_UBO bp_material_ubo_;
    std::string diffuse_texName;
};

struct oVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription() {
        return VkVertexInputBindingDescription{0, sizeof(oVertex), VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
        constexpr VkVertexInputAttributeDescription position_attribute_description = {
            0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(oVertex, position)
        };

        constexpr VkVertexInputAttributeDescription normal_attribute_description = {
            1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(oVertex, normal)
        };

        constexpr VkVertexInputAttributeDescription color_attribute_description = {
            2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(oVertex, texCoord)
        };

        return {position_attribute_description, normal_attribute_description, color_attribute_description};
    }

    bool operator==(const oVertex &other) const {
        return position == other.position && normal == other.normal && texCoord == other.texCoord;
    }
};

struct VertexHash {
    size_t operator()(const oVertex &vertex) const {
        const size_t h1 = std::hash<float>()(vertex.position[0]);
        const size_t h2 = std::hash<float>()(vertex.position[1]);
        const size_t h3 = std::hash<float>()(vertex.position[2]);
        const size_t h4 = std::hash<float>()(vertex.normal[0]);
        const size_t h5 = std::hash<float>()(vertex.normal[1]);
        const size_t h6 = std::hash<float>()(vertex.normal[2]);
        const size_t h7 = std::hash<float>()(vertex.texCoord[0]);
        const size_t h8 = std::hash<float>()(vertex.texCoord[1]);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6) ^ (h8 << 7);
    }
};

class ObjectComponent : public Component{

public:
    ObjectComponent(const char *obj, const char *basedir, Component* parent, VulkanRenderer* renderer ) : Component(parent),
        obj_(obj), basedir_(basedir), vk_renderer_(renderer) {
        loadObj();
        buffer_ = vk_renderer_->CreateVertexBuffer(getOVertices());
        index_buffer_ = vk_renderer_->CreateIndexBuffer(getIndices());
        for (const auto &texture: getTextures())
            texture_handles_.push_back(vk_renderer_->CreateTexture(texture.c_str()));
    };

    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime) override;
    void Render() const override;

    std::vector<oVertex> getOVertices() {
        return vertices_;
    };

    std::vector<Mesh> getMeshes() const {
        return meshes_;
    }

    std::vector<Material_UBO> getMaterialUBOs() const {
        std::vector<Material_UBO> materialUBOs;
        materialUBOs.reserve(materials_.size());
for (const auto& [bp_material_ubo_, diffuse_texName]: materials_)
            materialUBOs.push_back(bp_material_ubo_);
        return materialUBOs;
    }

    std::vector<std::string> getTextures() {
        std::vector<std::string> textures;
        textures.reserve(materials_.size());
for (const auto& [bp_material_ubo_, diffuse_texName]: materials_) {
            textures.push_back(diffuse_texName);
        }
        return textures;
    }

    [[nodiscard]] std::vector<std::uint32_t> getIndices() const {
        std::vector<std::uint32_t> indices;
        for (const auto &mesh: meshes_)
            for (const auto vertex: mesh.indices)
                indices.push_back(vertex);
        return indices;
    }

private:
    const char *obj_;
    const char *basedir_;
    std::vector<oVertex> vertices_;
    std::vector<material> materials_;
    std::vector<Mesh> meshes_;
    void loadObj();
    VulkanRenderer* vk_renderer_;
    BufferHandle buffer_;
    BufferHandle index_buffer_;
    mutable std::vector<TextureHandle> texture_handles_;
};

