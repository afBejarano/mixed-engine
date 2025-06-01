//
// Created by Yibuz Pokopodrozo on 2025-05-30.
//

#include <Trackball.h>
#include <Camera.h>

Trackball::Trackball(GLFWwindow* window, Camera* camera, VulkanRenderer* renderer)
    : window(window), camera(camera), renderer(renderer), mouseDown(false) {
    SetWindowDimensions();
    spdlog::info("Trackball initialized");
}

Trackball::~Trackball() = default;

void Trackball::SetWindowDimensions() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    const float halfWidth = static_cast<float>(width) * 0.5f;
    const float halfHeight = static_cast<float>(height) * 0.5f;

    invNDC = glm::mat4(
        glm::vec4(1.0f / halfWidth, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, -1.0f / halfHeight, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f)
    );
}

void Trackball::SetInitialView(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up) {
    initialEye = eye;
    initialAt = at;
    initialUp = up;
    initialView = glm::lookAt(eye, at, up);
    mouseRotationMatrix = glm::mat4(1.0f);

    cameraForward = -glm::normalize(eye - at);
    cameraRight = glm::normalize(glm::cross(cameraForward, up));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));

    if (camera) camera->LookAt(eye, at, up);
}

void Trackball::Reset() {
    mouseRotationMatrix = glm::mat4(1.0f);
    if (camera) {
        camera->LookAt(initialEye, initialAt, initialUp);
        if (renderer) renderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix(), initialEye);
    }

    cameraForward = -glm::normalize(initialEye - initialAt);
    cameraRight = glm::normalize(glm::cross(cameraForward, initialUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

void Trackball::HandleEvents() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!mouseDown) OnLeftMouseDown(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
        else OnMouseMove(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && mouseDown)
        OnLeftMouseUp(static_cast<uint32_t>(xpos), static_cast<uint32_t>(ypos));
}

glm::vec3 Trackball::GetMouseVector(uint32_t x, uint32_t y) const {
    const glm::vec4 mousePos(static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f);
    auto v = glm::vec3(invNDC * mousePos);

    v.x = -v.x;

    if (const float d2 = v.x * v.x + v.y * v.y; d2 <= radius * 0.5f) v.z = std::sqrt(radius - d2);
    else {
        const float d = std::sqrt(d2);
        v.z = (radius * 0.5f) / d;
    }

    const glm::vec3 cameraSpaceVector =
        v.x * cameraRight +
        v.y * cameraUp +
        v.z * cameraForward;

    return glm::normalize(cameraSpaceVector);
}

void Trackball::OnLeftMouseDown(const uint32_t x, const uint32_t y) {
    mouseDown = true;
    beginV = GetMouseVector(x, y);
}

void Trackball::OnLeftMouseUp(uint32_t x, uint32_t y) {
    mouseDown = false;
}

void Trackball::UpdateCameraBasis(const glm::vec3& newEye) {
    cameraForward = -glm::normalize(newEye - initialAt);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

void Trackball::OnMouseMove(const uint32_t x, const uint32_t y) {
    if (!mouseDown) return;

    endV = GetMouseVector(x, y);

    glm::vec3 rotAxis = glm::cross(beginV, endV);
    float rotAngle = glm::acos(glm::clamp(glm::dot(beginV, endV), -1.0f, 1.0f));

    rotAngle *= -0.9f;

    if (const float axisLength = glm::length(rotAxis); axisLength > 1e-5f
        && std::abs(rotAngle) > 1e-5f
        && std::abs(rotAngle) < glm::pi<float>() * 0.5f) {
        rotAxis = rotAxis / axisLength;

        const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotAngle, rotAxis);

        mouseRotationMatrix = rotation * mouseRotationMatrix;

        if (camera) {
            const glm::vec3 viewDir = initialEye - initialAt;
            const float distance = glm::length(viewDir);

            const glm::vec3 rotatedDir = glm::vec3(mouseRotationMatrix * glm::vec4(glm::normalize(viewDir), 0.0f));
            const glm::vec3 newEye = initialAt + rotatedDir * distance;

            UpdateCameraBasis(newEye);

            camera->LookAt(newEye, initialAt, cameraUp);

            if (renderer) renderer->SetViewProjection(camera->GetViewMatrix(), camera->GetProjectionMatrix(), newEye);
        }
    }

    beginV = endV;
}