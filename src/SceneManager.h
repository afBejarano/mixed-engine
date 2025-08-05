//
// Created by andre on 13/04/2025.
//
#pragma once

#include <Camera.h>
#include <Trackball.h>
#include <render/Renderer.h>
#include <render/Scene.h>
#include <window/Window.h>
#include <yaml-cpp/node/node.h>

struct CameraAction {
    int key;
    glm::vec3 vector;
    float speed;
    bool modifiesCameraTarget;
};

class SceneManager {
public:
    explicit SceneManager(RendererType render_type_);
    ~SceneManager();
    void Run();
    bool Initialize(const std::string& name_, int width_, int height_);
    void GetEvents();

    enum SCENE_NUMBER {
        SCENE0 = 0,
        SCENE1 = 1,
        SCENE2 = 2,
        SCENE3 = 3,
        SCENE4 = 4,
        SCENE5 = 5,
        SCENE6 = 6
    };

    enum CAMERA_TYPE {
        TRACKBALL = 0,
        CAMERA = 1
    };

    void ChangeScene(SCENE_NUMBER scene_);

private:
    RendererType renderType;
    Window* window{};
    Scene* currentScene{};
    class Timer* timer{};
    Camera* camera;
    Trackball* trackball;

    glm::vec3 camPos{};
    glm::vec3 camTarget{};
    glm::vec3 camUp{};

    CAMERA_TYPE camType;

    int currentSceneNumber{};

    Renderer* renderer{};
    unsigned int fps{};
    bool isRunning{};
    void BuildScene(SCENE_NUMBER scene_);

    void LoadCamera(const YAML::Node &node);

    void LoadPerspective(const YAML::Node &node) const;

    void LoadState(const YAML::Node &node, Scene *scene, VulkanRenderer *vr);

    Scene* LoadScene(const std::string &name_);
};
