cmake_minimum_required(VERSION 3.30)

project(MixedEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include(cmake/Shaders.cmake)

include(FetchContent)

include_directories(external)

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
        imgui
        GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
        GIT_TAG "v1.92.0"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm glfw spdlog imgui)

set(IMGUI_SOURCE_DIR ${imgui_SOURCE_DIR})
set(IMGUI_BACKENDS_DIR ${IMGUI_SOURCE_DIR}/backends)


set(IMGUI_CORE_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
)

file(GLOB_RECURSE MixedEngineSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp"
        "${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp"
)

add_executable(MixedEngine ${MixedEngineSources} ${IMGUI_CORE_SOURCES} external/tiny_obj_loader.h)

target_link_libraries(MixedEngine PRIVATE glm::glm glfw Vulkan::Vulkan spdlog::spdlog)

target_compile_definitions(MixedEngine PRIVATE TINYOBJLOADER_IMPLEMENTATION)

target_include_directories(MixedEngine PRIVATE "src"
        ${IMGUI_SOURCE_DIR}
        ${IMGUI_BACKENDS_DIR}
)

target_compile_features(MixedEngine PRIVATE cxx_std_20)

target_precompile_headers(MixedEngine PRIVATE "src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
)

add_shaders(MixedEngineShaders ${ShaderSources})
add_dependencies(MixedEngine MixedEngineShaders)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
