//
// Created by andre on 13/04/2025.
//

#include <SceneManager.h>

#include <render/Scene.h>
#include <rapidxml-1.13/rapidxml.hpp>
#include <rapidxml-1.13/rapidxml_utils.hpp>

#include <Camera.h>
#include <Trackball.h>

#include <components/TransformComponent.h>

SceneManager::SceneManager(RendererType render_type_) : renderType(render_type_), camera(nullptr), trackball(nullptr) {}

SceneManager::~SceneManager() {
    delete trackball;
    delete camera;
    delete currentScene;
    delete window;
    delete renderer;
}

void SceneManager::Run() {
    while (!glfwWindowShouldClose(window->getGLFWwindow())) {
        glfwPollEvents();
        if (trackball) {
            trackball->HandleEvents(); // Handle trackball events
        }
        if (renderType == RendererType::VULKAN) {
            VulkanRenderer *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
            for (auto key: vRenderer->shaders_)
                if (glfwGetKey(window->getGLFWwindow(), key.first) == GLFW_PRESS)
                    vRenderer->HandleShaderSwitch(key.first);
            if (vRenderer->BeginFrame()) {
                currentScene->Render();
                vRenderer->EndFrame();
            };
        }
    }
}

bool SceneManager::Initialize(const std::string &name_, const int width_, const int height_) {
    window = new Window(name_.c_str(), width_, height_, false);
    renderer = new VulkanRenderer(window);
    if (renderType == RendererType::VULKAN) {
        VulkanRenderer *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
        vRenderer->shaders_ = {
            {GLFW_KEY_1, "shaders/nopost.frag.spv"},
            {GLFW_KEY_2, "shaders/grayscale.frag.spv"},
            {GLFW_KEY_3, "shaders/colorReduction.frag.spv"},
            {GLFW_KEY_4, "shaders/scanlines.frag.spv"},
            {GLFW_KEY_5, "shaders/pixelation.frag.spv"},
            {GLFW_KEY_6, "shaders/retro.frag.spv"},
            {GLFW_KEY_7, "shaders/crt.frag.spv"},
            {GLFW_KEY_8, "shaders/chromab.frag.spv"},
            {GLFW_KEY_9, "shaders/vignette.frag.spv"},
            {GLFW_KEY_0, "shaders/brightness.frag.spv"},
            {GLFW_KEY_P, "shaders/bloom.frag.spv"},
            {GLFW_KEY_G, "shaders/glitch.frag.spv"},
            {GLFW_KEY_D, "shaders/dream.frag.spv"},
        };
    }
    camera = new Camera(); // Create camera
    trackball = new Trackball(window->getGLFWwindow(), camera, dynamic_cast<VulkanRenderer *>(renderer));
    // Create trackball with renderer
    currentScene = LoadScene("./assets/scenes/Scene2.xml");

    return true;
}

void SceneManager::GetEvents() {}

void SceneManager::ChangeScene(SCENE_NUMBER scene_) {}

void SceneManager::BuildScene(SCENE_NUMBER scene_) {}

glm::vec3 GetVector(rapidxml::xml_node<> *node, const std::string &namex = "x", const std::string &namey = "y",
                    const std::string &namez = "z") {
    return {
        std::stof(node->first_attribute(namex.c_str())->value()),
        std::stof(node->first_attribute(namey.c_str())->value()),
        std::stof(node->first_attribute(namez.c_str())->value())
    };
}
glm::vec4 GetVector4(rapidxml::xml_node<> *node, const std::string &namex = "x", const std::string &namey = "y",
                    const std::string &namez = "z", const std::string &namew = "w") {
    return {
        std::stof(node->first_attribute(namex.c_str())->value()),
        std::stof(node->first_attribute(namey.c_str())->value()),
        std::stof(node->first_attribute(namez.c_str())->value()),
        std::stof(node->first_attribute(namew.c_str())->value())
    };
}

void LoadActors(VulkanRenderer *vRenderer, const rapidxml::xml_node<> *baseNode, Scene *scene) {
    for (const rapidxml::xml_node<> *node = baseNode->first_node(); node; node = node->next_sibling()) {
        auto *actor = new Actor(nullptr);
        for (const auto *node2 = node->first_node(); node2; node2 = node2->next_sibling()) {
            if (std::string(node2->name()) == "ObjectComponent")
                actor->AddComponent<ObjectComponent>(node2->first_attribute("obj")->value(),
                                                     node2->first_attribute("basedir")->value(), actor, vRenderer);
            else if (std::string(node2->name()) == "TransformComponent") {
                auto *transform = node2->first_node("Position");
                auto position = GetVector(transform);

                auto *orientation = node2->first_node("Orientation");
                auto v_orientation = glm::radians(GetVector(orientation));

                auto *scale = node2->first_node("Scale");
                auto v_scale = GetVector(scale);

                actor->AddComponent<TransformComponent, Component *, glm::vec3, glm::quat, glm::vec3>(
                    actor, std::move(position), glm::quat(v_orientation), std::move(v_scale));
            }
        }
        scene->AddActor(actor);
    }
}

Scene *SceneManager::LoadScene(const std::string &name_) {
    rapidxml::file xmlFile(name_.c_str());
    rapidxml::xml_document doc;
    doc.parse<0>(xmlFile.data());

    auto *baseNode = doc.first_node();

    auto *scene = new Scene(renderer);

    if (renderer->getRendererType() == RendererType::VULKAN) {
        auto *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
        auto state_node = baseNode->first_node("State");
        auto camera_node = state_node->first_node("Camera");
        auto skybox_node = state_node->first_node("Skybox");
        auto lights_node = baseNode->first_node("Lights");
        std::array<const char *, 6> names;
        LightUBO lightUBOs[10];

        glm::vec2 size = vRenderer->GetWindowSize();
        glm::vec3 eye = GetVector(camera_node->first_node("Eye"));
        glm::vec3 center = GetVector(camera_node->first_node("Center"));
        glm::vec3 up = GetVector(camera_node->first_node("Up"));
        glm::vec3 pers = GetVector(state_node->first_node("Perspective"), "fov", "zNear", "zFar");

        int index = 0;
        for (auto node = skybox_node->first_node(); node; node = node->next_sibling()) {
            names[index] = node->first_attribute("path")->value();
            index++;
        }

        scene->global_lighting_ = new GlobalLighting();

        index = 0;
        for (auto node = lights_node->first_node(); node; node = node->next_sibling()) {
            glm::vec4 pos = GetVector4(node->first_node("Position"));
            glm::vec4 diff = GetVector4(node->first_node("Diffuse"));
            scene->global_lighting_->lights[index] = {pos, diff};
            index++;
        }

        scene->global_lighting_->numLights = index;

        camera->Perspective(glm::radians(pers.x), size.x / size.y, pers.y, pers.z);
        camera->LookAt(eye, center, up);
        trackball->SetInitialView(eye, center, up);

        // Set view and projection in renderer
        vRenderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix(), eye);

        LoadActors(vRenderer, baseNode->first_node("Actors"), scene);
        vRenderer->cubemap_ = names;
        vRenderer->CreateSkyboxResources();
    }

    return scene;
}
