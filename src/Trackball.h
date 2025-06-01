//
// Created by Yibuz Pokopodrozo on 2025-05-30.
//

#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "render/VulkanRenderer.h"

class Camera;  // Forward declaration

class Trackball {
public:
    explicit Trackball(GLFWwindow* window, Camera* camera = nullptr, VulkanRenderer* renderer = nullptr);
    ~Trackball();
    Trackball(const Trackball&) = delete;
    Trackball(Trackball&&) = delete;
    Trackball& operator=(const Trackball&) = delete;
    Trackball& operator=(Trackball&&) = delete;

    // Get rotation matrices
    [[nodiscard]] glm::mat4 getMatrix4() const { return mouseRotationMatrix; }
    [[nodiscard]] glm::mat3 getMatrix3() const { return glm::mat3(mouseRotationMatrix); }
    
    // Main update function
    void HandleEvents();
    
    // Window resize handling
    void setWindowDimensions();
    
    // Camera control
    void setCamera(Camera* camera_) { camera = camera_; }
    void setRenderer(VulkanRenderer* renderer_) { renderer = renderer_; }
    void setInitialView(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up);
    
    // Reset rotation
    void reset();

private:
    GLFWwindow* window;
    Camera* camera;
    VulkanRenderer* renderer;
    bool mouseDown;
    glm::mat4 mouseRotationMatrix = glm::mat4(1.0f);
    glm::mat4 invNDC = glm::mat4(1.0f);
    glm::vec3 beginV, endV;
    float radius = 0.8f; // Trackball radius in NDC
    
    // Camera state
    glm::vec3 initialEye{0.0f};
    glm::vec3 initialAt{0.0f};
    glm::vec3 initialUp{0.0f, 1.0f, 0.0f};
    glm::mat4 initialView = glm::mat4(1.0f);

    // Camera basis vectors
    glm::vec3 cameraRight{1.0f, 0.0f, 0.0f};   // x-axis
    glm::vec3 cameraUp{0.0f, 1.0f, 0.0f};      // y-axis
    glm::vec3 cameraForward{0.0f, 0.0f, -1.0f}; // negative z-axis

    [[nodiscard]] glm::vec3 getMouseVector(uint32_t x, uint32_t y) const;
    void onLeftMouseDown(uint32_t x, uint32_t y);
    void onLeftMouseUp(uint32_t x, uint32_t y);
    void onMouseMove(uint32_t x, uint32_t y);
    void updateCameraBasis(const glm::vec3& newEye);
};

#endif

