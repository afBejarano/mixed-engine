cmake_minimum_required(VERSION 3.30)

project(MixedEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY "https://github.com/g-truc/glm.git"
        GIT_TAG "1.0.1"
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.4"
        GIT_SHALLOW TRUE
)

fetchcontent_makeavailable(glm glfw)

add_executable(MixedEngine src/main.cpp
        src/SceneManager.cpp
        src/SceneManager.h
        src/render/Renderer.h
        src/render/VulkanRenderer.cpp
        src/render/VulkanRenderer.h
)

target_link_libraries(MixedEngine PRIVATE glm::glm glfw Vulkan::Vulkan)