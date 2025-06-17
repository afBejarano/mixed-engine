//
// Created by andre on 11/05/2025.
//

#pragma once

bool streq(const char* a, const char* b);
std::vector<std::uint8_t> ReadFile(const std::filesystem::path& shader_path);

inline bool LayerMatchesName(const char *layer_name, const VkLayerProperties &properties) {
    return streq(layer_name, properties.layerName);
}

inline bool IsLayerSupported(std::vector<VkLayerProperties> layers, const char *layer_name) {
    return std::ranges::any_of(layers, std::bind_front(&LayerMatchesName, layer_name));
}

inline bool VulkanRenderer::AreAllLayersSupported(const std::vector<const char *> &extensions) {
    return std::ranges::all_of(extensions, std::bind_front(IsLayerSupported, GetSupportedValidationLayers()));
}

inline bool ExtensionMatchesName(const char *extension_name, const VkExtensionProperties &extension) {
    return streq(extension_name, extension.extensionName);
}

inline bool IsExtensionSupported(std::vector<VkExtensionProperties> extensions, const char *extension_name) {
    return std::ranges::any_of(extensions, std::bind_front(&ExtensionMatchesName, extension_name));
}

inline bool IsDeviceExtensionWithinList(const std::vector<VkExtensionProperties> &extensions, const char *extension_name) {
    return std::ranges::any_of(extensions, [extension_name](const VkExtensionProperties &property) {
        return streq(extension_name, property.extensionName);
    });
}

inline bool IsMailboxPresent(const VkPresentModeKHR &present) {
    return present == VK_PRESENT_MODE_MAILBOX_KHR;
}

inline void CheckExit(VkResult res, const std::string& message) {
    if (res != VK_SUCCESS) {
        spdlog::error(message);
        exit(EXIT_FAILURE);
    }
}

inline void CheckExcept(VkResult res, const std::string& message) {
    if (res != VK_SUCCESS)
        throw std::runtime_error(message);
}


inline std::vector<glm::vec3> CreateSkyboxVertices() {
    return {
                {-1.0f, 1.0f, -1.0f},
                {-1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {-1.0f, 1.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
            };
}

inline std::vector<uint32_t> CreateSkyboxIndices() {
    return {
        0, 1, 3, 3, 1, 2,
        4, 5, 0, 0, 5, 1,
        3, 2, 7, 7, 2, 6,
        4, 0, 7, 7, 0, 3,
        1, 5, 2, 2, 5, 6,
        7, 6, 4, 4, 6, 5
    };
}

inline bool IsRgbaTypeFormat(const VkSurfaceFormatKHR &format) {
    return format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB;
}

inline bool IsSrgbColorSpace(const VkSurfaceFormatKHR &format) {
    return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

inline bool IsCorrectFormat(const VkSurfaceFormatKHR &format) {
    return IsRgbaTypeFormat(format) && IsSrgbColorSpace(format);
}


