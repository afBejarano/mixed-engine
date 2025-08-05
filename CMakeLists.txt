cmake_minimum_required(VERSION 3.30)

project(MixedEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include(cmake/Shaders.cmake)

# Include directories for bootstrapped dependencies
include_directories(deps)
include_directories(deps/src)

# Add subdirectories for bootstrapped libraries
add_subdirectory(deps/src/glm)
add_subdirectory(deps/src/glfw)
add_subdirectory(deps/src/spdlog)
add_subdirectory(deps/src/yaml-cpp)

# ImGui setup
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_SOURCE_DIR}/backends)

set(IMGUI_CORE_SOURCES
        ${IMGUI_SOURCE_DIR}/imgui.cpp
        ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
        ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
        ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
)

file(GLOB_RECURSE MixedEngineSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp"
        "${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp"
)

add_executable(MixedEngine ${MixedEngineSources} ${IMGUI_CORE_SOURCES} deps/src/tiny_obj_loader/tiny_obj_loader.h)

target_link_libraries(MixedEngine PRIVATE glm glfw Vulkan::Vulkan spdlog::spdlog yaml-cpp)

target_compile_definitions(MixedEngine PRIVATE TINYOBJLOADER_IMPLEMENTATION)

target_include_directories(MixedEngine PRIVATE "src"
        ${IMGUI_SOURCE_DIR}
        ${IMGUI_BACKENDS_DIR}
        "deps/src/tiny_obj_loader"
        "deps/src/stb"
        "deps/src/rapidxml"
        "deps/src/yaml-cpp/include"
)

target_compile_features(MixedEngine PRIVATE cxx_std_20)

target_precompile_headers(MixedEngine PRIVATE "src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
        "shaders/*.vert"
        "shaders/*.frag"
)

add_shaders(MixedEngineShaders ${ShaderSources})
add_dependencies(MixedEngine MixedEngineShaders)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
