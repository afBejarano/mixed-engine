//
// Created by andre on 13/04/2025.
//
#pragma once

#include "render/Renderer.h"
#include "window/Window.h"
#include "render/Scene.h"


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

    void ChangeScene(SCENE_NUMBER scene_);

private:
    RendererType renderType;
    Window* window;
    Scene* currentScene;
    class Timer* timer;

    Renderer* renderer;
    unsigned int fps;
    bool isRunning;
    void BuildScene(SCENE_NUMBER scene_);

    std::vector<ObjectComponent> components_;
};
