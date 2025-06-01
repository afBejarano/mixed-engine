//
// Created by andre on 13/04/2025.
//

#pragma once

#include <window/Window.h>

enum class RendererType {
    NONE,
    OPENGL,
    VULKAN,
    DIRECTX11,
    DIRECTX12
};

class Renderer {
public:
    Renderer(Window *window, RendererType render_type): renderer(nullptr), rendererType(render_type), window(window) {}
    virtual ~Renderer() = default;
    virtual bool OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void Render() = 0;
    [[nodiscard]] RendererType getRendererType() const { return rendererType; }
    void setRendererType(RendererType rendererType_) { rendererType = rendererType_; }

protected:
    Renderer *renderer;
    RendererType rendererType;
    Window *window;
};

