//
// Created by andre on 6/05/2025.
//

#include "VulkanRenderer.h"

#include "Utilities.h"

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                           VkDebugUtilsMessengerEXT pDebugMessenger,
                                                           const VkAllocationCallbacks* pAllocator) {
    auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pDebugMessenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* user_data) {
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

bool LayerMatchesName(const char* layer_name, const VkLayerProperties& properties) {
    return streq(layer_name, properties.layerName);
}

bool IsLayerSupported(std::vector<VkLayerProperties> layers, const char* layer_name) {
    return std::ranges::any_of(layers, std::bind_front(&LayerMatchesName, layer_name));
}

bool VulkanRenderer::AreAllLayersSupported(const std::vector<const char*>& extensions) {
    return std::ranges::all_of(extensions, std::bind_front(IsLayerSupported, GetSupportedValidationLayers()));
}

bool ExtensionMatchesName(const char* extension_name, const VkExtensionProperties& extension) {
    return streq(extension_name, extension.extensionName);
}

bool IsExtensionSupported(std::vector<VkExtensionProperties> extensions, const char* extension_name) {
    return std::ranges::any_of(extensions, std::bind_front(&ExtensionMatchesName, extension_name));
}

bool VulkanRenderer::AreAllExtensionsSupported(const std::vector<const char*>& extensions) {
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

std::vector<const char*> VulkanRenderer::GetSuggestedInstanceExtensions() {
    std::uint32_t extension_count = 0;
    const char** extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    std::vector<const char*> result;
    result.reserve(extension_count);
    for (int i = 0; i < extension_count; i++) {
        result.push_back(extension_names[i]);
    }
    return result;
}

std::vector<const char*> VulkanRenderer::GetRequiredInstanceExtensions() const {
    std::vector<const char*> suggested_extensions = GetSuggestedInstanceExtensions();
    std::vector<const char*> required_extensions(suggested_extensions.size());
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

    std::vector<const char*> requiredExtensions = GetRequiredInstanceExtensions();
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
        create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();
    } else {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = nullptr;
    }

    VkResult result = vkCreateInstance(&create_info, nullptr, &vk_instance_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create instance!");
        exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::CreateSurface() {
    VkResult result = glfwCreateWindowSurface(vk_instance_, window->getGLFWwindow(), nullptr, &vk_surface_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create window surface!");
        exit(EXIT_FAILURE);
    }
}

QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device) const {
    std::uint32_t graphics_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &graphics_families, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(graphics_families);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &graphics_families, queue_families.data());

    auto graphics_family_it = std::ranges::find_if(queue_families, [](const VkQueueFamilyProperties& props) {
        return props.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    });

    QueueFamilyIndices indices;
    indices.graphicsFamily = graphics_family_it - queue_families.begin();

    for (std::uint32_t i = 0; i < queue_families.size(); ++i) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk_surface_, &present_support);
        if (present_support) {
            indices.presentFamily = i;
            break;
        }
    }

    return indices;
}

SwapchainSupportCapabilities VulkanRenderer::FindSwapChainSupport(VkPhysicalDevice device) const {
    SwapchainSupportCapabilities details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk_surface_, &details.capabilities_khr);

    std::uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface_, &format_count, nullptr);
    details.formats_khr.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk_surface_, &format_count, details.formats_khr.data());

    std::uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface_, &present_mode_count, nullptr);
    details.present_modes_khr.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk_surface_, &present_mode_count,
                                              details.present_modes_khr.data());

    return details;
}

std::vector<VkExtensionProperties> VulkanRenderer::GetDeviceAvailableExtensions(VkPhysicalDevice device) {
    std::uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

    return extensions;
}

bool IsDeviceExtensionWithinList(const std::vector<VkExtensionProperties>& extensions, const char* extension_name) {
    return std::ranges::any_of(extensions, [extension_name](const VkExtensionProperties& property) {
        return streq(extension_name, property.extensionName);
    });
}

bool VulkanRenderer::AreAllDeviceExtensionsSupported(VkPhysicalDevice device) {
    std::vector<VkExtensionProperties> extensions = GetDeviceAvailableExtensions(device);
    return std::ranges::all_of(deviceExtensions, std::bind_front(IsDeviceExtensionWithinList, extensions));
}

bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice device) const {
    const QueueFamilyIndices indices = FindQueueFamilies(device);

    return indices.isComplete() && AreAllDeviceExtensionsSupported(device) && FindSwapChainSupport(device).IsValid();
}

std::vector<VkPhysicalDevice> VulkanRenderer::GetPhysicalDevices() const {
    std::uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, nullptr);

    if (device_count == 0) return {};

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, devices.data());

    return devices;
}

void VulkanRenderer::PickPhysicalDevice() {
    auto devices = GetPhysicalDevices();

    std::erase_if(devices, std::not_fn(std::bind_front(&VulkanRenderer::IsDeviceSuitable, this)));
    if (devices.empty()) {
        spdlog::error("Failed to find a suitable GPU!");
        std::exit(EXIT_FAILURE);
    }

    vk_physical_device_ = devices[0];
}

void VulkanRenderer::CreateLogicalDeviceAndQueues() {
    QueueFamilyIndices indices = FindQueueFamilies(vk_physical_device_);

    if (!indices.isComplete()) {
        spdlog::error("Failed to find valid queue families!");
        std::exit(EXIT_FAILURE);
    }

    std::set uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    std::float_t priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (const auto unique_queue_family : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, unique_queue_family, 1, &priority
            };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, static_cast<uint32_t>(queueCreateInfos.size()),
            queueCreateInfos.data(), 0, nullptr, static_cast<uint32_t>(deviceExtensions.size()),
            deviceExtensions.data(),
            &deviceFeatures
        };

    VkResult result = vkCreateDevice(vk_physical_device_, &deviceCreateInfo, nullptr, &vk_device_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create logical device!");
        std::exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(vk_device_, indices.graphicsFamily.value(), 0, &vk_graphics_queue_);
    vkGetDeviceQueue(vk_device_, indices.presentFamily.value(), 0, &vk_present_queue_);
}


bool IsRgbaTypeFormat(const VkSurfaceFormatKHR& format) {
    return format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB;
}

bool IsSrgbColorSpace(const VkSurfaceFormatKHR& format) {
    return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

bool IsCorrectFormat(const VkSurfaceFormatKHR& format) {
    return IsRgbaTypeFormat(format) && IsSrgbColorSpace(format);
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8A8_SNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    if (auto it = std::ranges::find_if(formats, IsCorrectFormat); it != formats.end()) {
        return *it;
    }
    return formats[0];
}

bool IsMailboxPresent(const VkPresentModeKHR& present) {
    return present == VK_PRESENT_MODE_MAILBOX_KHR;
}

VkPresentModeKHR VulkanRenderer::ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> present_modes) {
    if (std::ranges::any_of(present_modes, IsMailboxPresent)) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    const glm::ivec2 size = window->GetFrameBufferSize();
    VkExtent2D actual_extend = {
            static_cast<std::uint32_t>(size.x),
            static_cast<std::uint32_t>(size.y)
        };

    actual_extend.width = std::clamp(actual_extend.width, capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actual_extend.height = std::clamp(actual_extend.height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);
    return actual_extend;
}

std::uint32_t VulkanRenderer::ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
    std::uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }
    return image_count;
}

void VulkanRenderer::CreateSwapChain() {
    SwapchainSupportCapabilities capabilities = FindSwapChainSupport(vk_physical_device_);

    vk_surface_format_ = ChooseSwapchainSurfaceFormat(capabilities.formats_khr);
    vk_present_mode_ = ChooseSwapchainPresentMode(capabilities.present_modes_khr);
    vk_extent_ = ChooseSwapchainExtent(capabilities.capabilities_khr);
    std::uint32_t image_count = ChooseImageCount(capabilities.capabilities_khr);

    VkSwapchainCreateInfoKHR create_info_khr = {};
    create_info_khr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info_khr.surface = vk_surface_;
    create_info_khr.minImageCount = image_count;
    create_info_khr.imageFormat = vk_surface_format_.format;
    create_info_khr.imageColorSpace = vk_surface_format_.colorSpace;
    create_info_khr.imageExtent = vk_extent_;
    create_info_khr.imageArrayLayers = 1;
    create_info_khr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info_khr.presentMode = vk_present_mode_;
    create_info_khr.preTransform = capabilities.capabilities_khr.currentTransform;
    create_info_khr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info_khr.clipped = VK_TRUE;
    create_info_khr.oldSwapchain = VK_NULL_HANDLE;

    QueueFamilyIndices indices = FindQueueFamilies(vk_physical_device_);

    if (indices.graphicsFamily != indices.presentFamily) {
        std::array<std::uint32_t, 2> family_indices = {
                indices.graphicsFamily.value(),
                indices.presentFamily.value()
            };

        create_info_khr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info_khr.queueFamilyIndexCount = family_indices.size();
        create_info_khr.pQueueFamilyIndices = family_indices.data();
    } else {
        create_info_khr.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult result = vkCreateSwapchainKHR(vk_device_, &create_info_khr, nullptr, &vk_swapchain_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create swapchain!");
        std::exit(EXIT_FAILURE);
    }

    std::uint32_t actual_image_count;
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &actual_image_count, nullptr);
    vk_swapchain_images_.resize(actual_image_count);
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &actual_image_count, vk_swapchain_images_.data());
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, const VkFormat format) const {
    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    VkImageView view;
    VkResult result = vkCreateImageView(vk_device_, &view_create_info, nullptr, &view);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create image view!");
        std::exit(EXIT_FAILURE);
    }
    return view;
}

void VulkanRenderer::CreateImageViews() {
    vk_swapchain_image_views_.resize(vk_swapchain_images_.size());

    auto image_view_it = vk_swapchain_image_views_.begin();
    for (VkImage image : vk_swapchain_images_) {
        *image_view_it = CreateImageView(image, vk_surface_format_.format);
        image_view_it = std::next(image_view_it);
    }
}


VulkanRenderer::VulkanRenderer(Window* window): Renderer(window) {
#if !defined(NDEBUG)
    validation_ = true;
#endif
    InitializeVulkan();
}

VulkanRenderer::~VulkanRenderer() {
    VulkanRenderer::OnDestroy();
}

bool VulkanRenderer::OnCreate() {
    return true;
}

void VulkanRenderer::OnDestroy() {
    if (vk_device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(vk_device_, nullptr);
    }

    if (vk_instance_ != VK_NULL_HANDLE) {
        if (vk_surface_ != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(vk_instance_, vk_surface_, VK_NULL_HANDLE);

        if (debug_messenger_ != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(vk_instance_, debug_messenger_, VK_NULL_HANDLE);

        vkDestroyInstance(vk_instance_, VK_NULL_HANDLE);
    }
}

void VulkanRenderer::Render() {
}

void VulkanRenderer::InitializeVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDeviceAndQueues();
    CreateSwapChain();
    CreateImageViews();
}
