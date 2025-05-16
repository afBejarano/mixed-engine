cmake_minimum_required(VERSION 3.30)

project(MixedEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include(cmake/Shaders.cmake)

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

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
        GIT_TAG "v1.15.2"
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        microsoft-gsl
        GIT_REPOSITORY "https://github.com/microsoft/GSL.git"
        GIT_TAG "v4.2.0"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm glfw spdlog microsoft-gsl)

file(GLOB_RECURSE MixedEngineSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(MixedEngine ${MixedEngineSources}
        src/precomp.h
        src/precomp.cpp
        src/SceneManager.cpp
        src/SceneManager.h
        src/render/Renderer.h
        src/render/VulkanRenderer.cpp
        src/render/VulkanRenderer.h
        #src/Actor.h
        src/Camera.cpp
        src/Camera.h
        src/CameraPosition.h
        src/Collider.h
        src/window/Window.cpp
        src/window/Window.h
        src/render/Scene.cpp
        src/render/Scene.h
        src/Utilities.cpp
        src/Utilities.h
        src/window/Timer.cpp
        src/window/Timer.h
        src/Component.h
        #src/Actor.cpp
        src/Debug.cpp
        src/Debug.h
        src/vertex.h
        src/BufferHandle.h
        src/UniformTransformations.h
        src/TextureHandle.h
        src/stb_image.cpp
        src/stb_image.h
)

target_link_libraries(MixedEngine PRIVATE glm::glm glfw Vulkan::Vulkan spdlog Microsoft.GSL::GSL)

target_include_directories(MixedEngine PRIVATE "src")

target_compile_features(MixedEngine PRIVATE cxx_std_20)

target_precompile_headers(MixedEngine PRIVATE "src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
)

add_shaders(MixedEngineShaders ${ShaderSources})
add_dependencies(MixedEngine MixedEngineShaders)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/textures/paving-stones.jpg" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")