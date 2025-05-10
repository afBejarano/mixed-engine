//
// Created by andre on 13/04/2025.
//

#pragma once
#include <string>

#include "GLFW/glfw3.h"

enum class RendererType {
    NONE,
    OPENGL,
    VULKAN,
    DIRECTX11,
    DIRECTX12
};

class Renderer {
public:
    Renderer(): renderer(nullptr), rendererType(RendererType::NONE) {}
    virtual ~Renderer() = default;
    virtual GLFWwindow *CreateWindow(std::string name_, int width_, int height_) = 0;
    virtual bool OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void Render() = 0;
    [[nodiscard]] RendererType getRendererType() const { return rendererType; }
    void setRendererType(RendererType rendererType_) { rendererType = rendererType_; }

protected:
    Renderer *renderer;
    RendererType rendererType;
};

