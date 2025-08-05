//
// Created by andre on 13/04/2025.
//

#include <SceneManager.h>

#include <render/Scene.h>
#include <yaml-cpp/yaml.h>

#include <Camera.h>
#include <MacTypes.h>
#include <Trackball.h>

#include <components/TransformComponent.h>

SceneManager::SceneManager(const RendererType render_type_) : renderType(render_type_), camera(nullptr), trackball(nullptr), camType(TRACKBALL) {}

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
        switch (camType) {
            case CAMERA: {
                float moveSpeed = 0.02f;
                float lookSpeed = 0.05f;
                glm::vec3 forward = normalize(camTarget - camPos);
                glm::vec3 right = normalize(cross(forward, camUp));

                const std::vector<CameraAction> actions = {
                    {GLFW_KEY_W, forward, moveSpeed, false},
                    {GLFW_KEY_S, -forward, moveSpeed, false},
                    {GLFW_KEY_A, -right, moveSpeed, false},
                    {GLFW_KEY_D, right, moveSpeed, false},

                    {GLFW_KEY_UP, -camUp, lookSpeed, true},
                    {GLFW_KEY_DOWN, camUp, lookSpeed, true},
                    {GLFW_KEY_LEFT, -right, lookSpeed, true},
                    {GLFW_KEY_RIGHT, right, lookSpeed, true}
                };

                for (const auto& action : actions) {
                    if (glfwGetKey(window->getGLFWwindow(), action.key) == GLFW_PRESS) {
                        if (action.modifiesCameraTarget) {
                            camTarget += action.vector * action.speed;
                        } else {
                            camPos += action.vector * action.speed;
                            camTarget += action.vector * action.speed;
                        }
                    }
                }
                camera->LookAt(camPos, camTarget, camUp);
                if (renderType == RendererType::VULKAN) {
                    const auto *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
                    vRenderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camPos);
                }
                break;
            }
            case TRACKBALL:
                trackball ? trackball->HandleEvents() : throw std::runtime_error("Trackball is not initialized");
                break;
            default:
                break;
        }

        if (renderType == RendererType::VULKAN) {
            auto *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);
            for (const auto&[key, shader]: vRenderer->shaders_)
                if (glfwGetKey(window->getGLFWwindow(), key) == GLFW_PRESS)
                    vRenderer->HandleShaderSwitch(key);
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
    camera = new Camera();
    if (renderType == RendererType::VULKAN)
        trackball = new Trackball(window->getGLFWwindow(), camera, dynamic_cast<VulkanRenderer *>(renderer));

    currentScene = LoadScene("./assets/scenes/Scene1.yml");

    return true;
}

void SceneManager::GetEvents() {}

void SceneManager::ChangeScene(SCENE_NUMBER scene_) {}

void SceneManager::BuildScene(SCENE_NUMBER scene_) {}

glm::vec3 LoadVec3(const YAML::Node &node, const std::string &x = "x", const std::string &y = "y", const std::string &z = "z") {
    return {
        node[x].as<float>(),
        node[y].as<float>(),
        node[z].as<float>()
    };
}

glm::vec4 LoadVec4(const YAML::Node &node, const std::string &x = "x", const std::string &y = "y", const std::string &z = "z", const std::string &w = "w") {
    return {
        node[x].as<float>(),
        node[y].as<float>(),
        node[z].as<float>(),
        node[w].as<float>()
    };
}

void LoadObjectComponent(Actor* actor, const VulkanRenderer* vr, const YAML::Node &node) {
    actor->AddComponent<ObjectComponent>(node["obj"].as<std::string>().c_str(), node["basedir"].as<std::string>().c_str(), actor, const_cast<VulkanRenderer*>(vr));
}

void LoadTransformComponent(Actor* actor, const YAML::Node &node) {
    glm::vec3 position = LoadVec3(node["position"]);
    const auto orientation = radians(LoadVec3(node["orientation"]));
    glm::vec3 scale = LoadVec3(node["scale"]);
    actor->AddComponent<TransformComponent>(actor, position,
        glm::quat(orientation), scale);
}

void SceneManager::LoadCamera(const YAML::Node &node) {
    camPos = LoadVec3(node["eye"]);
    camTarget = LoadVec3(node["center"]);
    camUp = LoadVec3(node["up"]);
    // For now, default to TRACKBALL since we can't easily convert string to enum
    camType = TRACKBALL;
}

void  SceneManager::LoadPerspective(const YAML::Node &node) const {
    const glm::vec3 perspective = LoadVec3(node, "fov", "zNear","zFar");
    const glm::vec2 size = window->GetFrameBufferSize();
    camera->Perspective(glm::radians(perspective.x), size.x / size.y, perspective.y, perspective.z);
    camera->LookAt(camPos, camTarget, camUp);
    trackball->SetInitialView(camPos, camTarget, camUp);
}

std::array<std::string, 6> LoadSkybox(const YAML::Node &node) {
    return {
        node["right"].as<std::string>(),
        node["left"].as<std::string>(),
        node["top"].as<std::string>(),
        node["bottom"].as<std::string>(),
        node["front"].as<std::string>(),
        node["back"].as<std::string>()
    };
}

void SceneManager::LoadState(const YAML::Node &node, Scene *scene, VulkanRenderer *vr) {
    LoadCamera(node["camera"]);
    LoadPerspective(node["perspective"]);
    vr->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camPos);
    vr->cubemap_ = LoadSkybox(node["skybox"]);
}

void LoadActors(const YAML::Node &node, Scene *scene, const VulkanRenderer *vr) {
    for (const auto& actor : node) {
        const auto newActor = new Actor(nullptr);
        scene->AddActor(newActor);
        if (actor["object"])
            LoadObjectComponent(newActor, vr, actor["object"]);
        if (actor["transform"])
            LoadTransformComponent(newActor, actor["transform"]);
    }
}

void LoadLights(const YAML::Node &node, Scene *scene) {
    scene->global_lighting_ = new GlobalLighting();
    int index = 0;
    for (const auto& lightNode : node) {
        if (lightNode["position"] && lightNode["diffuse"]) {
            scene->global_lighting_->lights[index++] = {LoadVec4(lightNode["position"]), LoadVec4(lightNode["diffuse"])};
        }
    }
    scene->global_lighting_->numLights = index;
}

std::unordered_map<int, std::string> LoadShaders(const YAML::Node &node) {
    std::unordered_map<int, std::string> shaders;
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
        shaders.emplace(it->first.as<char>(), it->second.as<std::string>());
    }
    return shaders;
}

Scene *SceneManager::LoadScene(const std::string &name_) {
    auto *scene = new Scene(renderer);

    YAML::Node config = YAML::LoadFile(name_)["scene"];

    if (renderer->getRendererType() == RendererType::VULKAN) {
        auto *vRenderer = dynamic_cast<VulkanRenderer *>(renderer);

        LoadState(config["state"], scene, vRenderer);
        LoadActors(config["actors"], scene, vRenderer);
        LoadLights(config["lights"], scene);
        
        vRenderer->shaders_ = LoadShaders(config["shaders"]);

        vRenderer->CreateSkyboxResources();
    }

    return scene;
}
