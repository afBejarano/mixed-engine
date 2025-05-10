//
// Created by andre on 6/05/2025.
//

#pragma once

#include "Renderer.h"
#include "window/Window.h"

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(Window *window);
    ~VulkanRenderer() override;

    VulkanRenderer(const VulkanRenderer &) = delete; /// Copy constructor
    VulkanRenderer(VulkanRenderer &&) = delete; /// Move constructor
    VulkanRenderer &operator=(const VulkanRenderer &) = delete; /// Copy operator
    VulkanRenderer &operator=(VulkanRenderer &&) = delete; /// Move operator

    bool OnCreate();
    void OnDestroy();
    void Render();

private:
    GLFWwindow *glfWwindow;
};
