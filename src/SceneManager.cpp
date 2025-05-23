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
                currentScene->Render();
                vRenderer->EndFrame();
            };
        }
    }
}

bool SceneManager::Initialize(const std::string& name_, const int width_, const int height_) {
    window = new Window(name_.c_str(), width_, height_, false);
    renderer = new VulkanRenderer(window);
    currentScene = new Scene(renderer);

    return true;
}

void SceneManager::GetEvents() {
}

void SceneManager::ChangeScene(SCENE_NUMBER scene_) {
}

void SceneManager::BuildScene(SCENE_NUMBER scene_) {
}
