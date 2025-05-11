//
// Created by andre on 6/05/2025.
//

#include "VulkanRenderer.h"

#include "Utilities.h"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                           VkDebugUtilsMessengerEXT pDebugMessenger,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pDebugMessenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                         void *user_data) {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error("Vulkan Validation: {}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("Vulkan Validation: {}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("Vulkan Validation: {}", pCallbackData->pMessage);
    } else {
        spdlog::debug("Vulkan Validation: {}", pCallbackData->pMessage);
    }
    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT GetCreateDebugMessengerInfo() {
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.pNext = nullptr;

    messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    messengerCreateInfo.pfnUserCallback = ValidationCallback;
    messengerCreateInfo.pUserData = nullptr;

    return messengerCreateInfo;
}

void VulkanRenderer::SetupDebugMessenger() {
    if (!validation_) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT info = GetCreateDebugMessengerInfo();
    VkResult result = vkCreateDebugUtilsMessengerEXT(vk_instance_, &info, nullptr, &debug_messenger_);
    if (result != VK_SUCCESS) spdlog::error("Failed to create debug messenger");
}

bool LayerMatchesName(const char *layer_name, const VkLayerProperties &properties) {
    return streq(layer_name, properties.layerName);
}

bool IsLayerSupported(std::vector<VkLayerProperties> layers, const char *layer_name) {
    return std::ranges::any_of(layers, std::bind_front(&LayerMatchesName, layer_name));
}

bool VulkanRenderer::AreAllLayersSupported(const std::vector<const char *> &extensions) {
    return std::ranges::all_of(extensions, std::bind_front(IsLayerSupported, GetSupportedValidationLayers()));
}

bool ExtensionMatchesName(const char *extension_name, const VkExtensionProperties &extension) {
    return streq(extension_name, extension.extensionName);
}

bool IsExtensionSupported(std::vector<VkExtensionProperties> extensions, const char *extension_name) {
    return std::ranges::any_of(extensions, std::bind_front(&ExtensionMatchesName, extension_name));
}

bool VulkanRenderer::AreAllExtensionsSupported(const std::vector<const char *> &extensions) {
    return std::ranges::all_of(extensions, std::bind_front(IsExtensionSupported, GetSupportedInstanceExtensions()));
}

std::vector<VkLayerProperties> VulkanRenderer::GetSupportedValidationLayers() {
    uint32_t propertyCount = 0;

    vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

    if (propertyCount == 0) return {};

    std::vector<VkLayerProperties> layers(propertyCount);
    vkEnumerateInstanceLayerProperties(&propertyCount, layers.data());

    return layers;
}

std::vector<VkExtensionProperties> VulkanRenderer::GetSupportedInstanceExtensions() {
    std::uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    if (extension_count == 0) { return {}; }

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    return extensions;
}

std::vector<const char *> VulkanRenderer::GetSuggestedInstanceExtensions() {
    std::uint32_t extension_count = 0;
    const char **extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    std::vector<const char *> result;
    result.reserve(extension_count);
for (int i = 0; i < extension_count; i++) {
        result.push_back(extension_names[i]);
    }
    return result;
}

std::vector<const char *> VulkanRenderer::GetRequiredInstanceExtensions() const {
    std::vector<const char *> suggested_extensions = GetSuggestedInstanceExtensions();
    std::vector<const char *> required_extensions(suggested_extensions.size());
    std::ranges::copy(suggested_extensions, required_extensions.begin());

    if (validation_) {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!AreAllExtensionsSupported(required_extensions)) {
        std::cout << "No supported extensions found" << std::endl;
        exit(EXIT_FAILURE);
    }

    return required_extensions;
}

void VulkanRenderer::CreateInstance() {
    if (!AreAllLayersSupported(validationLayers))
        validation_ = false;

    std::vector<const char *> requiredExtensions = GetRequiredInstanceExtensions();
    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "Vulkan Project", VK_MAKE_VERSION(0, 0, 1), "Mixed Engine",
        VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_2
    };

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = GetCreateDebugMessengerInfo();

    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &app_info, 0, nullptr,
        static_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data()
    };

    if (validation_) {
        create_info.pNext = &debug_create_info;
        create_info.enabledLayerCount =  static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();
    }

    VkResult result = vkCreateInstance(&create_info, nullptr, &vk_instance_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create instance!");
        exit(EXIT_FAILURE);
    }
}

VulkanRenderer::VulkanRenderer(Window *window): Renderer(window) {
#if !defined(NDEBUG)
    validation_ = true;
#endif
    InitializeVulkan();
}


VulkanRenderer::~VulkanRenderer() {}

bool VulkanRenderer::OnCreate() {
    return true;
}

void VulkanRenderer::OnDestroy() {}

void VulkanRenderer::Render() {}

void VulkanRenderer::InitializeVulkan() {
    CreateInstance();
}
