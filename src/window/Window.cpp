//
// Created by andre on 2025-05-10.
//

#include "Window.h"
#include <iostream>
#include <ostream>

Window::Window(const char *title, const int width, const int height, bool fullscreen) : width(width), height(height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        glm::ivec2 size = glm::ivec2(width, height);
        if (monitor != nullptr)
            glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &size.x, &size.y);
        window = glfwCreateWindow(size.x, size.y, title, glfwGetPrimaryMonitor(), nullptr);
    } else {
        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    }

    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLFWwindow* Window::GetWindow() {
    return window;
}
