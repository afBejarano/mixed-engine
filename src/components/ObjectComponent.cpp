//
// Created by andre on 2025-05-21.
//

#include <components/ObjectComponent.h>

#include <tiny_obj_loader.h>
#include <components/Actor.h>
#include <components/TransformComponent.h>

bool ObjectComponent::OnCreate() {
    return true;
}

void ObjectComponent::OnDestroy() {
    if (buffer_.buffer != VK_NULL_HANDLE) {
        vk_renderer_->DestroyBuffer(buffer_);
    }
    if (index_buffer_.buffer != VK_NULL_HANDLE) {
        vk_renderer_->DestroyBuffer(index_buffer_);
    }
    
    // Destroy all texture handles
    for (auto& texture_handle : texture_handles_) {
        if (texture_handle.image != VK_NULL_HANDLE) {
            vk_renderer_->DestroyTexture(texture_handle);
        }
    }
    texture_handles_.clear();
}

void ObjectComponent::Update(float deltaTime) {
}

void ObjectComponent::Render() const {
    vk_renderer_->RenderModel(buffer_, index_buffer_, getMeshes(), texture_handles_, getMaterialUBOs(),
                              dynamic_cast<Actor*>(parent)->GetComponent<TransformComponent>()->GetTransformMatrix());
}

void ObjectComponent::loadObj() {
    tinyobj::attrib_t attrib_t;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    bool ret = LoadObj(&attrib_t, &shapes, &materials, &err, obj_, basedir_, true);
    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    for (unsigned int i = 0; i < attrib_t.vertices.size(); i += 3)
        vertices.emplace_back(attrib_t.vertices[i + 0], attrib_t.vertices[i + 1], attrib_t.vertices[i + 2]);

    for (unsigned int i = 0; i < attrib_t.normals.size(); i += 3)
        normals.emplace_back(attrib_t.normals[i + 0], attrib_t.normals[i + 1], attrib_t.normals[i + 2]);

    for (unsigned int i = 0; i < attrib_t.texcoords.size(); i += 2)
        texCoords.emplace_back(attrib_t.texcoords[i + 0], 1 - (attrib_t.texcoords[i + 1]));

    std::unordered_map<oVertex, int, VertexHash> indices;

    for (const auto& [name, mesh] : shapes) {
        Mesh m;
        m.materialId = mesh.material_ids[0];
        for (int i = 0; i < mesh.indices.size(); i++) {
            oVertex v{
                    vertices.at(mesh.indices[i].vertex_index),
                    normals.at(mesh.indices[i].normal_index),
                    texCoords.at(mesh.indices[i].texcoord_index)
                };
            if (auto index = indices.find(v); index != indices.end()) {
                m.indices.push_back(index->second);
            } else {
                m.indices.push_back(vertices_.size());

                indices.emplace(v, vertices_.size());

                vertices_.push_back(v);
            }
        }
        meshes_.push_back(m);
    }

    for (const auto& shape : materials) {
        Material_UBO material_ubo;
        material_ubo.ambient = glm::vec3{shape.ambient[0], shape.ambient[1], shape.ambient[2]};
        material_ubo.diffuse = glm::vec3{shape.diffuse[0], shape.diffuse[1], shape.diffuse[2]};
        material_ubo.specular = glm::vec3{shape.specular[0], shape.specular[1], shape.specular[2]};
        // Ensure shininess is not too low
        material_ubo.shininess = std::max(shape.shininess, 32.0f);  // Default to 32 if too low
        
        materials_.push_back(material{
            material_ubo,
            basedir_ + shape.diffuse_texname.substr(2)
        });
    }
}
