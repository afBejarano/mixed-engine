//
// Created by Yibuz Pokopodrozo on 2025-05-30.
//

#include "Trackball.h"
#include "Camera.h"

Trackball::Trackball(GLFWwindow* window_, Camera* camera_, VulkanRenderer* renderer_) 
    : window(window_), camera(camera_), renderer(renderer_), mouseDown(false) {
    setWindowDimensions();
    spdlog::info("Trackball initialized");
}

Trackball::~Trackball() = default;

void Trackball::setWindowDimensions() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    // Store window dimensions for mouse position normalization
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Simpler NDC transformation
    invNDC = glm::mat4(
        glm::vec4(1.0f/halfWidth, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, -1.0f/halfHeight, 0.0f, 0.0f),  // Flip Y coordinate
        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f)  // Adjusted translation
    );
    
    spdlog::debug("Window dimensions set: {}x{}", width, height);
}

void Trackball::setInitialView(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up) {
    initialEye = eye;
    initialAt = at;
    initialUp = up;
    initialView = glm::lookAt(eye, at, up);
    mouseRotationMatrix = glm::mat4(1.0f);
    
    // Initialize camera basis vectors
    cameraForward = -glm::normalize(eye - at);  // Camera looks along negative z
    cameraRight = glm::normalize(glm::cross(cameraForward, up));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
    
    if (camera) {
        camera->LookAt(eye, at, up);
        spdlog::info("Initial view set - Eye: ({},{},{}), At: ({},{},{})", 
            eye.x, eye.y, eye.z, at.x, at.y, at.z);
    }
}

void Trackball::reset() {
    mouseRotationMatrix = glm::mat4(1.0f);
    if (camera) {
        camera->LookAt(initialEye, initialAt, initialUp);
        if (renderer) {
            renderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
        spdlog::info("Camera reset to initial position");
    }
    
    // Reset camera basis vectors
    cameraForward = -glm::normalize(initialEye - initialAt);
    cameraRight = glm::normalize(glm::cross(cameraForward, initialUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

void Trackball::HandleEvents() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!mouseDown) {
            onLeftMouseDown(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
        } else {
            onMouseMove(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && mouseDown) {
        onLeftMouseUp(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
    }
}

glm::vec3 Trackball::getMouseVector(uint32_t x, uint32_t y) const {
    // Convert to normalized device coordinates (-1 to 1)
    glm::vec4 mousePos(static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f);
    glm::vec3 v = glm::vec3(invNDC * mousePos);
    
    // Invert X coordinate only (Y is now correct)
    v.x = -v.x;
    
    // Project onto sphere
    float d2 = v.x * v.x + v.y * v.y;
    if (d2 <= radius * 0.5f) {
        v.z = std::sqrt(radius - d2);
    } else {
        float d = std::sqrt(d2);
        v.z = (radius * 0.5f) / d;
    }
    
    // Transform the point from screen space to camera space
    glm::vec3 cameraSpaceVector = 
        v.x * cameraRight +
        v.y * cameraUp +
        v.z * cameraForward;
    
    spdlog::debug("Mouse vector: ({},{},{})", v.x, v.y, v.z);
    return glm::normalize(cameraSpaceVector);
}

void Trackball::onLeftMouseDown(uint32_t x, uint32_t y) {
    mouseDown = true;
    beginV = getMouseVector(x, y);
    spdlog::info("Mouse down at: ({},{})", x, y);
}

void Trackball::onLeftMouseUp(uint32_t x, uint32_t y) {
    mouseDown = false;
    spdlog::info("Mouse up at: ({},{})", x, y);
}

void Trackball::updateCameraBasis(const glm::vec3& newEye) {
    // Update camera basis vectors based on new eye position
    cameraForward = -glm::normalize(newEye - initialAt);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

void Trackball::onMouseMove(uint32_t x, uint32_t y) {
    if (!mouseDown) return;
    
    endV = getMouseVector(x, y);
    
    // Calculate rotation axis and angle
    glm::vec3 rotAxis = glm::cross(beginV, endV);
    float rotAngle = glm::acos(glm::clamp(glm::dot(beginV, endV), -1.0f, 1.0f));
    
    // Scale the rotation angle for smoother movement and invert direction
    rotAngle *= -0.5f;  // Negative to invert rotation direction
    
    // Only rotate if we have a valid axis and a reasonable angle
    float axisLength = glm::length(rotAxis);
    if (axisLength > 1e-5f && std::abs(rotAngle) > 1e-5f && std::abs(rotAngle) < glm::pi<float>() * 0.5f) {
        rotAxis = rotAxis / axisLength;  // Normalize without creating new vector
        
        // Create rotation matrix - start with identity matrix
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotAngle, rotAxis);
        
        // Apply new rotation to accumulated rotation
        mouseRotationMatrix = rotation * mouseRotationMatrix;
        
        if (camera) {
            // Calculate new eye position
            glm::vec3 viewDir = initialEye - initialAt;
            float distance = glm::length(viewDir);  // Preserve distance from look-at point
            
            // Apply rotation to view direction
            glm::vec3 rotatedDir = glm::vec3(mouseRotationMatrix * glm::vec4(glm::normalize(viewDir), 0.0f));
            glm::vec3 newEye = initialAt + rotatedDir * distance;
            
            // Update camera basis vectors before calculating new up vector
            updateCameraBasis(newEye);
            
            // Update camera
            camera->LookAt(newEye, initialAt, cameraUp);
            
            // Update renderer's view matrix
            if (renderer) {
                renderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix());
            }
            
            spdlog::debug("Camera updated - Eye: ({},{},{}), Angle: {}", 
                newEye.x, newEye.y, newEye.z, rotAngle);
        }
    }
    
    beginV = endV;
}