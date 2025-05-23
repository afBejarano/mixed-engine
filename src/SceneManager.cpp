//
// Created by andre on 13/04/2025.
//

#include "SceneManager.h"

#include "render/Scene.h"

SceneManager::SceneManager(RendererType render_type_) : renderType(render_type_) {
}

SceneManager::~SceneManager() = default;

void SceneManager::Run() {
    while (!glfwWindowShouldClose(window->getGLFWwindow())) {
        glfwPollEvents();
        if (renderType == RendererType::VULKAN) {
            VulkanRenderer* vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
            if (vRenderer->BeginFrame()) {
                for (auto component : components_) {
                    component.Render();
                }
                vRenderer->EndFrame();
            };
        }
    }
}

bool SceneManager::Initialize(const std::string& name_, const int width_, const int height_) {
    window = new Window(name_.c_str(), width_, height_, false);
    renderer = new VulkanRenderer(window);
    currentScene = new Scene(renderer);
    if (renderType == RendererType::VULKAN) {
        VulkanRenderer* vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
        float radians = glm::radians(90.0f); // Convert degrees to radians
        float halfAngle = radians / 2.0f;
        float w = cos(halfAngle);
        float x = sin(halfAngle);
        float y = 0.0f; // Rotation is around X axis, so y and z are zero
        float z = 0.0f;
        components_.push_back(ObjectComponent("./assets/Skull/12140_Skull_v3_L2.obj", "./assets/Skull/", nullptr,
                                              vRenderer,
                                              TransformComponent(nullptr, {0.0f, 0.0f, 0.0f}, glm::quat(w, x, y, z),
                                                                 {0.1f, 0.1f, 0.1f})));
        glm::vec2 size = {width_, height_};
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), size.x / size.y, 0.1f, 100.0f);
        vRenderer->SetViewProjection(view, proj);
    }
    return true;
}

void SceneManager::GetEvents() {
}

void SceneManager::ChangeScene(SCENE_NUMBER scene_) {
}

void SceneManager::BuildScene(SCENE_NUMBER scene_) {
}
