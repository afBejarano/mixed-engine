//
// Created by andre on 2025-05-10.
//

#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window(const char* title, int width, int height, bool fullscreen);
    ~Window();
    [[nodiscard]] GLFWwindow *getGLFWwindow() const {return window;};
    static void processInput(GLFWwindow *window);
    GLFWwindow *GetWindow();
private:
    GLFWwindow *window;
    int width, height;
};