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

FetchContent_MakeAvailable(glm glfw spdlog)

file(GLOB_RECURSE MixedEngineSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(MixedEngine ${MixedEngineSources})

target_link_libraries(MixedEngine PRIVATE glm::glm glfw Vulkan::Vulkan spdlog)

target_include_directories(MixedEngine PRIVATE "src")

target_compile_features(MixedEngine PRIVATE cxx_std_20)

target_precompile_headers(MixedEngine PRIVATE "src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
)

add_shaders(MixedEngineShaders ${ShaderSources})
add_dependencies(MixedEngine MixedEngineShaders)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/paving-stones.jpg" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/textures/")