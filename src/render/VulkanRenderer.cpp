//
// Created by andre on 6/05/2025.
//

#include <render/VulkanRenderer.h>

#include <TextureHandle.h>
#include <Utilities.h>

#include <CameraPosition.h>
#include <UniformTransformations.h>

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
    VkResult result = vkCreateDebugUtilsMessengerEXT(vk_instance_, &info, nullptr, &vk_debug_messenger_);
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

    auto graphics_family_it = std::ranges::find_if(queue_families, [](const VkQueueFamilyProperties &props) {
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

bool IsDeviceExtensionWithinList(const std::vector<VkExtensionProperties> &extensions, const char *extension_name) {
    return std::ranges::any_of(extensions, [extension_name](const VkExtensionProperties &property) {
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

    for (const auto unique_queue_family: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, unique_queue_family, 1, &priority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures required_features = {};
    required_features.depthBounds = true;
    required_features.depthClamp = true;

    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(), 0, nullptr, static_cast<uint32_t>(deviceExtensions.size()),
        deviceExtensions.data(),
        &required_features
    };

    VkResult result = vkCreateDevice(vk_physical_device_, &deviceCreateInfo, nullptr, &vk_device_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create logical device!");
        std::exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(vk_device_, indices.graphicsFamily.value(), 0, &vk_graphics_queue_);
    vkGetDeviceQueue(vk_device_, indices.presentFamily.value(), 0, &vk_present_queue_);
}


bool IsRgbaTypeFormat(const VkSurfaceFormatKHR &format) {
    return format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB;
}

bool IsSrgbColorSpace(const VkSurfaceFormatKHR &format) {
    return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

bool IsCorrectFormat(const VkSurfaceFormatKHR &format) {
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

bool IsMailboxPresent(const VkPresentModeKHR &present) {
    return present == VK_PRESENT_MODE_MAILBOX_KHR;
}

VkPresentModeKHR VulkanRenderer::ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> present_modes) {
    if (std::ranges::any_of(present_modes, IsMailboxPresent)) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
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

std::uint32_t VulkanRenderer::ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities) {
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

VkImageView VulkanRenderer::CreateImageView(VkImage image, const VkFormat format,
                                            VkImageAspectFlags aspect_flags) const {
    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = aspect_flags;
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

    for (VkImage image: vk_swapchain_images_) {
        *image_view_it = CreateImageView(image, vk_surface_format_.format, VK_IMAGE_ASPECT_COLOR_BIT);
        image_view_it = std::next(image_view_it);
    }
}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<std::uint8_t> &buffer) const {
    if (buffer.empty()) return VK_NULL_HANDLE;

    VkShaderModuleCreateInfo vk_shader_module_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, buffer.size(),
        reinterpret_cast<const uint32_t *>(buffer.data())
    };

    VkShaderModule vk_shader_module;
    VkResult result = vkCreateShaderModule(vk_device_, &vk_shader_module_info, nullptr, &vk_shader_module);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create shader module!");
        return VK_NULL_HANDLE;
    }

    return vk_shader_module;
}

void VulkanRenderer::CreateGraphicsPipeline() {
    main_pipeline_helper_ = {
        {"shaders/basic.vert.spv", "shaders/basic.frag.spv"}, {oVertex::GetBindingDescription()},
        oVertex::GetAttributeDescriptions(), VK_CULL_MODE_NONE, {true, true, VK_COMPARE_OP_LESS},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)}},
        {vk_uniform_set_layout_, vk_uniform_bp_set_layout_, vk_texture_set_layout_, vk_lights_set_layout_}
    };
    main_pipeline_helper_.color_blend_attachment = new VkPipelineColorBlendAttachmentState{
        VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    CreatePipeline(main_pipeline_helper_);
}

void VulkanRenderer::CreatePipeline(PipelineHelper &pipeline_helper) {
    VkShaderModule vertex_shader = CreateShaderModule(ReadFile(pipeline_helper.shaders[0]));
    VkShaderModule fragment_shader = CreateShaderModule(ReadFile(pipeline_helper.shaders[1]));

    if (vertex_shader == VK_NULL_HANDLE || fragment_shader == VK_NULL_HANDLE) {
        spdlog::error("Failed finding shaders!");
        std::exit(EXIT_FAILURE);
    }

    VkPipelineShaderStageCreateInfo vertex_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertex_shader,
        "main"
    };

    VkPipelineShaderStageCreateInfo fragment_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader,
        "main"
    };

    std::array stage_infos = {vertex_info, fragment_info};

    std::array dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr, 0, dynamic_states.size(), dynamic_states.data()
    };

    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();

    VkPipelineViewportStateCreateInfo viewport_state_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr, 0, 1, &viewport, 1, &scissor
    };

    auto vertex_binding_description = pipeline_helper.vertex_input_binding_description;
    auto vertex_attribute_description = pipeline_helper.vertex_input_attribute_description;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0,
        static_cast<uint32_t>(vertex_binding_description.size()), vertex_binding_description.data(),
        static_cast<uint32_t>(vertex_attribute_description.size()), vertex_attribute_description.data()
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE,
        VK_POLYGON_MODE_FILL, pipeline_helper.cull_mode, VK_FRONT_FACE_CLOCKWISE, VK_FALSE
    };
    rasterization_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
        pipeline_helper.depth_helper.enable_depth_testing, pipeline_helper.depth_helper.enable_depth_writing,
        pipeline_helper.depth_helper.compare_op,VK_FALSE, VK_FALSE
    };

    depthStencil_create_info.minDepthBounds = 0.0f;
    depthStencil_create_info.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr, 0, VK_FALSE, VK_LOGIC_OP_NO_OP, 1,
        pipeline_helper.color_blend_attachment ? pipeline_helper.color_blend_attachment : &color_blend_attachment
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
        static_cast<uint32_t>(pipeline_helper.descriptor_set_layouts.size()),
        pipeline_helper.descriptor_set_layouts.data(),
        static_cast<uint32_t>(pipeline_helper.push_constant_ranges.size()), pipeline_helper.push_constant_ranges.data()
    };

    if (VkResult result = vkCreatePipelineLayout(vk_device_, &pipeline_layout_info, nullptr,
                                                 &pipeline_helper.pipeline_layout);
        result != VK_SUCCESS) {
        spdlog::error("failed to create pipeline layout!");
        std::exit(EXIT_FAILURE);
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, stage_infos.size(), stage_infos.data(),
        &vertex_input_info, &input_assembly_info, nullptr, &viewport_state_info, &rasterization_state_info,
        &multisample_info, &depthStencil_create_info, &color_blend_state, &dynamic_state_info,
        pipeline_helper.pipeline_layout, vk_render_pass_, 0
    };

    VkResult pipeline_result = vkCreateGraphicsPipelines(vk_device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                                         &pipeline_helper.pipeline);
    if (pipeline_result != VK_SUCCESS) {
        spdlog::error("failed to create pipeline!");
        std::exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(vk_device_, vertex_shader, nullptr);
    vkDestroyShaderModule(vk_device_, fragment_shader, nullptr);
}

VkViewport VulkanRenderer::GetViewport() const {
    const VkViewport viewport = {
        0, 0, static_cast<float>(vk_extent_.width),
        static_cast<float>(vk_extent_.height), 0.0f, 1.0f
    };
    return viewport;
}

VkRect2D VulkanRenderer::GetScissor() const {
    const VkRect2D scissor = {0, 0, vk_extent_};
    return scissor;
}

void VulkanRenderer::CreateRenderPass() {
    // Color attachment
    VkAttachmentDescription color_attachment{};
    color_attachment.format = vk_surface_format_.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = FindDepthFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Attachment references
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Create subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    // Subpass dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create render pass
    std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_device_, &render_pass_info, nullptr, &vk_render_pass_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanRenderer::CreateFramebuffers() {
    vk_swapchain_framebuffers_.resize(vk_swapchain_image_views_.size());

    for (std::uint32_t i = 0; i < vk_swapchain_image_views_.size(); i++) {
        std::array attachments = std::array{vk_swapchain_image_views_[i], depth_texture_.image_view};
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = vk_render_pass_;
        framebuffer_info.attachmentCount = attachments.size();
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = vk_extent_.width;
        framebuffer_info.height = vk_extent_.height;
        framebuffer_info.layers = 1;

        VkResult result = vkCreateFramebuffer(vk_device_, &framebuffer_info, nullptr, &vk_swapchain_framebuffers_[i]);
        if (result != VK_SUCCESS) {
            spdlog::error("failed to create framebuffer!");
            std::exit(EXIT_FAILURE);
        }
    }
}

void VulkanRenderer::CreateCommandPool() {
    QueueFamilyIndices indices = FindQueueFamilies(vk_physical_device_);
    VkCommandPoolCreateInfo pool_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, indices.graphicsFamily.value()
    };

    VkResult result = vkCreateCommandPool(vk_device_, &pool_info, nullptr, &vk_command_pool_);
    if (result != VK_SUCCESS) {
        spdlog::error("failed to create command pool!");
        std::exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo alloc_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr, vk_command_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    };

    VkResult result = vkAllocateCommandBuffers(vk_device_, &alloc_info, &vk_command_buffer_);
    if (result != VK_SUCCESS) {
        spdlog::error("failed to allocate command buffers!");
        std::exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::BeginCommands() {
    vkResetCommandBuffer(vk_command_buffer_, 0);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    VkResult result = vkBeginCommandBuffer(vk_command_buffer_, &begin_info);
    if (result != VK_SUCCESS) throw std::runtime_error("failed to begin command buffer commands");

    // First render pass (scene)
    std::array<VkClearValue, 2> clear_values = {}; // Zero initialize all values

    // Initialize color clear value
    clear_values[0].color = {0.2f, 0.3f, 0.3f, 1.0f};

    // Initialize depth clear value
    VkClearDepthStencilValue depth_clear = {};
    depth_clear.depth = 1.0f;
    depth_clear.stencil = 0;
    clear_values[1].depthStencil = depth_clear;

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = post_processing_.render_pass; // Use the main render pass for the first pass
    render_pass_info.framebuffer = post_processing_.framebuffer;
    render_pass_info.renderArea = {{0, 0}, vk_extent_};
    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(vk_command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    RenderSkybox();

    vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, main_pipeline_helper_.pipeline);
    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();

    vkCmdSetViewport(vk_command_buffer_, 0, 1, &viewport);
    vkCmdSetScissor(vk_command_buffer_, 0, 1, &scissor);
}

void VulkanRenderer::EndCommands() const {
    vkCmdEndRenderPass(vk_command_buffer_);

    // Second render pass (post-processing)
    std::array<VkClearValue, 2> clear_values = {}; // Zero initialize all values
    clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = vk_render_pass_;
    render_pass_info.framebuffer = vk_swapchain_framebuffers_[current_image_index_];
    render_pass_info.renderArea = {{0, 0}, vk_extent_};
    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(vk_command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, post_processing_.pipeline.pipeline);
    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            post_processing_.pipeline.pipeline_layout, 0, 1,
                            &post_processing_.descriptor_set, 0, nullptr);

    // Update time push constant
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();
    
    vkCmdPushConstants(vk_command_buffer_, 
                      post_processing_.pipeline.pipeline_layout,
                      VK_SHADER_STAGE_FRAGMENT_BIT,
                      0, sizeof(float), &time);

    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();
    vkCmdSetViewport(vk_command_buffer_, 0, 1, &viewport);
    vkCmdSetScissor(vk_command_buffer_, 0, 1, &scissor);

    vkCmdDraw(vk_command_buffer_, 3, 1, 0, 0); // Draw fullscreen triangle

    vkCmdEndRenderPass(vk_command_buffer_);

    VkResult result = vkEndCommandBuffer(vk_command_buffer_);
    if (result != VK_SUCCESS) throw std::runtime_error("failed to end command buffer commands");
}

void VulkanRenderer::CreateSignals() {
    VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    if (vkCreateSemaphore(vk_device_, &semaphore_info, nullptr, &vk_image_available_signal_) != VK_SUCCESS) {
        spdlog::error("failed to locate image available signal!");
        std::exit(EXIT_FAILURE);
    }

    if (vkCreateSemaphore(vk_device_, &semaphore_info, nullptr, &vk_render_finished_signal_) != VK_SUCCESS) {
        spdlog::error("failed to render finished signal!");
        std::exit(EXIT_FAILURE);
    }

    VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};

    if (vkCreateFence(vk_device_, &fence_info, nullptr, &vk_still_rendering_fence_) != VK_SUCCESS) {
        spdlog::error("failed to create fence!");
        std::exit(EXIT_FAILURE);
    }
}

bool VulkanRenderer::BeginFrame() {
    vkWaitForFences(vk_device_, 1, &vk_still_rendering_fence_, VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        vk_device_,
        vk_swapchain_,
        UINT64_MAX,
        vk_image_available_signal_,
        VK_NULL_HANDLE,
        &current_image_index_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw std::runtime_error("failed to acquire next image!");

    vkResetFences(vk_device_, 1, &vk_still_rendering_fence_);
    BeginCommands();

    // Set model matrix for subsequent rendering
    SetModelMatrix(glm::mat4(1.0f));

    return true;
}

void VulkanRenderer::EndFrame() {
    EndCommands();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &vk_image_available_signal_;
    submit_info.pWaitDstStageMask = &wait_stage_flags;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_command_buffer_;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vk_render_finished_signal_;

    VkResult submit_result = vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, vk_still_rendering_fence_);
    if (submit_result != VK_SUCCESS) throw std::runtime_error("failed to submit queue!");

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_render_finished_signal_;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain_;
    present_info.pImageIndices = &current_image_index_;

    VkResult result = vkQueuePresentKHR(vk_present_queue_, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) throw std::runtime_error("failed to present swapchain images!");
}

std::uint32_t VulkanRenderer::FindMemoryType(const std::uint32_t memory_type_bits,
                                             const VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &memory_properties);
    const std::vector memory_types(memory_properties.memoryTypes,
                                   memory_properties.memoryTypes + memory_properties.memoryTypeCount);

    for (uint32_t i = 0; i < memory_types.size(); i++) {
        const bool passes_filter = memory_type_bits & (1 << i);
        const bool has_property_flags = memory_types[i].propertyFlags & properties;

        if (passes_filter && has_property_flags) return i;
    }

    throw std::runtime_error("failed to find memory type!");
}

BufferHandle VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                          VkMemoryPropertyFlags properties) {
    BufferHandle buffer_handle = {};

    VkBufferCreateInfo buffer_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0,
        size, usage, VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult result = vkCreateBuffer(vk_device_, &buffer_info, nullptr, &buffer_handle.buffer);
    if (result != VK_SUCCESS) throw std::runtime_error("failed to create vertex buffer!");

    VkMemoryRequirements memory_requirements = {};
    vkGetBufferMemoryRequirements(vk_device_, buffer_handle.buffer, &memory_requirements);

    std::uint32_t chosen_memory_type = FindMemoryType(memory_requirements.memoryTypeBits, properties);

    VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr, memory_requirements.size, chosen_memory_type
    };

    VkResult allocation_result = vkAllocateMemory(vk_device_, &memory_allocate_info, nullptr, &buffer_handle.memory);

    if (allocation_result != VK_SUCCESS) throw std::runtime_error("failed to allocate buffer memory!");

    vkBindBufferMemory(vk_device_, buffer_handle.buffer, buffer_handle.memory, 0);

    return buffer_handle;
}

BufferHandle VulkanRenderer::CreateIndexBuffer(std::vector<uint32_t> indices) {
    VkDeviceSize buffer_size = sizeof(uint32_t) * indices.size();
    BufferHandle buffer_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(vk_device_, buffer_handle.memory, 0, buffer_size, 0, &data);
    std::memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(vk_device_, buffer_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_region = {0, 0, buffer_size};
    vkCmdCopyBuffer(transient_commands, buffer_handle.buffer, gpu_handle.buffer, 1, &copy_region);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(buffer_handle);

    return gpu_handle;
}

BufferHandle VulkanRenderer::CreateVertexBuffer(std::vector<oVertex> vertices) {
    VkDeviceSize buffer_size = sizeof(oVertex) * vertices.size();
    BufferHandle buffer_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(vk_device_, buffer_handle.memory, 0, buffer_size, 0, &data);
    std::memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(vk_device_, buffer_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_region = {0, 0, buffer_size};
    vkCmdCopyBuffer(transient_commands, buffer_handle.buffer, gpu_handle.buffer, 1, &copy_region);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(buffer_handle);

    return gpu_handle;
}

BufferHandle VulkanRenderer::CreateVertexBuffer(const std::vector<glm::vec3> &vertices) {
    VkDeviceSize buffer_size = sizeof(glm::vec3) * vertices.size();
    BufferHandle buffer_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(vk_device_, buffer_handle.memory, 0, buffer_size, 0, &data);
    std::memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(vk_device_, buffer_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(
        buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_region = {0, 0, buffer_size};
    vkCmdCopyBuffer(transient_commands, buffer_handle.buffer, gpu_handle.buffer, 1, &copy_region);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(buffer_handle);

    return gpu_handle;
}

void VulkanRenderer::DestroyBuffer(const BufferHandle buffer_handle) const {
    if (vk_device_ == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle(vk_device_);

    if (buffer_handle.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vk_device_, buffer_handle.buffer, nullptr);
    }

    if (buffer_handle.memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_device_, buffer_handle.memory, nullptr);
    }
}

void VulkanRenderer::RenderBuffer(BufferHandle buffer_handle, std::uint32_t vertex_count) {
    VkDeviceSize offset = 0;
    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, main_pipeline_helper_.pipeline_layout,
                            0, 1,
                            &vk_uniform_set_, 0, nullptr);
    vkCmdBindVertexBuffers(vk_command_buffer_, 0, 1, &buffer_handle.buffer, &offset);
    vkCmdDraw(vk_command_buffer_, vertex_count, 1, 0, 0);
    SetModelMatrix(glm::mat4(1.0f));
}

void VulkanRenderer::RenderIndexedBuffer(BufferHandle vertex_buffer_handle, BufferHandle index_buffer_handle,
                                         std::uint32_t index_count, std::int32_t index_offset) {
    VkDeviceSize offset = 0;
    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, main_pipeline_helper_.pipeline_layout,
                            0, 2,
                            std::array{vk_uniform_set_, vk_bp_set_}.data(), 0, nullptr);
    vkCmdBindVertexBuffers(vk_command_buffer_, 0, 1, &vertex_buffer_handle.buffer, &offset);
    vkCmdBindIndexBuffer(vk_command_buffer_, index_buffer_handle.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(vk_command_buffer_, index_count, 1, 0, index_offset, 0);
    SetModelMatrix(glm::mat4(1.0f));
}

void VulkanRenderer::RenderModel(BufferHandle vertex_buffer, BufferHandle index_buffer, const std::vector<Mesh> &meshes,
                                 std::vector<TextureHandle> &textures, std::vector<Material_UBO> material_ubos,
                                 const glm::mat4 &modelMatrix) {
    int offset = 0;
    VkDeviceSize dOffset = 0;
    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, main_pipeline_helper_.pipeline_layout,
                            0, 2,
                            std::array{vk_uniform_set_, vk_bp_set_}.data(), 0, VK_NULL_HANDLE);

    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, main_pipeline_helper_.pipeline_layout,
                            3, 1,
                            std::array{vk_lights_set_}.data(), 0, VK_NULL_HANDLE);
    vkCmdBindVertexBuffers(vk_command_buffer_, 0, 1, &vertex_buffer.buffer, &dOffset);
    vkCmdBindIndexBuffer(vk_command_buffer_, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    SetModelMatrix(modelMatrix);
    for (const auto &[indices, materialId]: meshes) {
        SetTexture(textures[materialId]);
        SetUbo(material_ubos[materialId]);
        vkCmdDrawIndexed(vk_command_buffer_, indices.size(), 1, offset, 0, 0);
        offset += static_cast<int>(indices.size());
    }
}

void VulkanRenderer::SetModelMatrix(const glm::mat4 &matrix) const {
    vkCmdPushConstants(vk_command_buffer_, main_pipeline_helper_.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(glm::mat4),
                       &matrix);
}

void VulkanRenderer::SetViewProjection(glm::mat4 matrix, glm::mat4 projection, glm::vec3 cameraPos) {
    UniformTransformations transformations{matrix, projection, cameraPos};
    std::memcpy(uniform_buffer_location_, &transformations, sizeof(UniformTransformations));
}

void VulkanRenderer::SetUbo(Material_UBO &material_ubos) const {
    memcpy(bp_buffer_location_, &material_ubos, sizeof(Material_UBO));
}

void VulkanRenderer::SetLightsUBO(GlobalLighting *global_lighting) {
    memcpy(global_lights_buffer_location_, global_lighting, sizeof(GlobalLighting));
}

VkCommandBuffer VulkanRenderer::BeginTransientCommandBuffer() {
    VkCommandBufferAllocateInfo alloc_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr, vk_command_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vk_device_, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void VulkanRenderer::EndTransientCommandBuffer(VkCommandBuffer command_buffer) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk_graphics_queue_);
    vkFreeCommandBuffers(vk_device_, vk_command_pool_, 1, &command_buffer);
}

void VulkanRenderer::CreateUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(UniformTransformations);
    uniform_buffer_ = CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(vk_device_, uniform_buffer_.memory, 0, buffer_size, 0, &uniform_buffer_location_);

    VkDeviceSize bp_size = sizeof(Material_UBO);
    bp_buffer_handle_ = CreateBuffer(bp_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(vk_device_, bp_buffer_handle_.memory, 0, bp_size, 0, &bp_buffer_location_);

    VkDeviceSize lights_size = sizeof(GlobalLighting);
    g_light_handle_ = CreateBuffer(lights_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(vk_device_, g_light_handle_.memory, 0, lights_size, 0, &global_lights_buffer_location_);
}

void VulkanRenderer::CreateDescriptorSetLayouts() {
    VkDescriptorSetLayoutBinding uniform_layout_binding = {
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL_GRAPHICS
    };

    VkDescriptorSetLayoutBinding uniform_bp_layout_binding = {
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding lights_layout_binding = {
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutCreateInfo uniform_layout_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &uniform_layout_binding
    };

    VkDescriptorSetLayoutCreateInfo bp_uniform_layout_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &uniform_bp_layout_binding
    };

    VkDescriptorSetLayoutCreateInfo lights_layout_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &lights_layout_binding
    };

    if (vkCreateDescriptorSetLayout(vk_device_, &uniform_layout_info, nullptr, &vk_uniform_set_layout_) != VK_SUCCESS) {
        spdlog::error("Failed to create uniform descriptor set layout!");
        exit(EXIT_FAILURE);
    }

    if (vkCreateDescriptorSetLayout(vk_device_, &bp_uniform_layout_info, nullptr, &vk_uniform_bp_set_layout_) !=
        VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
        std::exit(EXIT_FAILURE);
    }

    if (vkCreateDescriptorSetLayout(vk_device_, &lights_layout_info, nullptr, &vk_lights_set_layout_) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
        std::exit(EXIT_FAILURE);
    }

    VkDescriptorSetLayoutBinding texture_layout_binding = {
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutCreateInfo texture_layout_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1, &texture_layout_binding
    };

    if (vkCreateDescriptorSetLayout(vk_device_, &texture_layout_info, nullptr, &vk_texture_set_layout_) != VK_SUCCESS) {
        spdlog::error("Failed to create texture descriptor set layout!");
        exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::CreateDescriptorPools() {
    VkDescriptorPoolSize uniform_pool_sizes = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3};

    VkDescriptorPoolCreateInfo pool_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr, 0, 3, 1, &uniform_pool_sizes
    };

    if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &vk_uniform_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create uniform pool!");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(vk_physical_device_, &device_properties);

    VkDescriptorPoolSize texture_pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024};

    VkDescriptorPoolCreateInfo texture_pool_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1024, 1, &texture_pool_size
    };

    if (vkCreateDescriptorPool(vk_device_, &texture_pool_info, nullptr, &vk_texture_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create texture pool!");
        exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::CreateDescriptorSets() {
    VkDescriptorSetAllocateInfo alloc_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_uniform_set_layout_
    };

    VkDescriptorSetAllocateInfo bp_descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_uniform_bp_set_layout_
    };

    VkDescriptorSetAllocateInfo lights_descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_lights_set_layout_
    };

    VkResult result = vkAllocateDescriptorSets(vk_device_, &lights_descriptor_set_allocate_info, &vk_lights_set_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
        exit(EXIT_FAILURE);
    }

    result = vkAllocateDescriptorSets(vk_device_, &alloc_info, &vk_uniform_set_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
        exit(EXIT_FAILURE);
    }

    result = vkAllocateDescriptorSets(vk_device_, &bp_descriptor_set_allocate_info, &vk_bp_set_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
        std::exit(EXIT_FAILURE);
    }

    VkDescriptorBufferInfo buffer_info = {uniform_buffer_.buffer, 0, sizeof(UniformTransformations)};

    VkWriteDescriptorSet descriptor_write = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, vk_uniform_set_, 0, 0,
        1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &buffer_info
    };

    VkDescriptorBufferInfo bp_descriptor_buffer_info = {bp_buffer_handle_.buffer, 0, sizeof(Material_UBO)};

    VkWriteDescriptorSet bp_descriptor_write = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, vk_bp_set_, 0, 0,
        1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &bp_descriptor_buffer_info
    };

    VkDescriptorBufferInfo gLights = {g_light_handle_.buffer, 0, sizeof(GlobalLighting)};

    VkWriteDescriptorSet gLightsWrite = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, vk_lights_set_, 0, 0,
        1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &gLights
    };

    std::array writes = {descriptor_write, bp_descriptor_write, gLightsWrite};

    vkUpdateDescriptorSets(vk_device_, writes.size(), writes.data(), 0, nullptr);
}

void VulkanRenderer::CreateTextureSampler() {
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.maxAnisotropy = 1.0f;

    VkResult result = vkCreateSampler(vk_device_, &sampler_info, nullptr, &vk_texture_sampler_);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create texture sampler!");
        exit(EXIT_FAILURE);
    }
}

void VulkanRenderer::CreateDepthResources() {
    VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    depth_texture_ = CreateImage({vk_extent_.width, vk_extent_.height}, depth_format,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    depth_texture_.image_view = CreateImageView(depth_texture_.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

TextureHandle VulkanRenderer::CreateTexture(const char *path) {
    glm::ivec2 image_extents;
    std::int32_t channels;
    std::vector<std::uint8_t> image_file_data = ReadFile(path);
    stbi_uc *pixel_data = stbi_load_from_memory(image_file_data.data(), static_cast<int>(image_file_data.size()),
                                                &image_extents.x, &image_extents.y, &channels, STBI_rgb_alpha);

    VkDeviceSize buffer_size = image_extents.x * image_extents.y * 4;
    BufferHandle staging_buffer = CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data_location;
    vkMapMemory(vk_device_, staging_buffer.memory, 0, buffer_size, 0, &data_location);
    std::memcpy(data_location, pixel_data, buffer_size);
    vkUnmapMemory(vk_device_, staging_buffer.memory);

    stbi_image_free(pixel_data);

    TextureHandle handle = CreateImage(image_extents, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                               VK_IMAGE_USAGE_SAMPLED_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    TransitionImageLayout(handle.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(staging_buffer.buffer, handle.image, image_extents);
    TransitionImageLayout(handle.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorSetAllocateInfo descriptor_set_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr, vk_texture_pool_, 1, &vk_texture_set_layout_
    };

    VkResult result = vkAllocateDescriptorSets(vk_device_, &descriptor_set_info, &handle.descriptor_set);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
        exit(EXIT_FAILURE);
    }

    handle.image_view = CreateImageView(handle.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    VkDescriptorImageInfo image_info = {
        vk_texture_sampler_, handle.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = handle.descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(vk_device_, 1, &descriptor_write, 0, nullptr);

    DestroyBuffer(staging_buffer);
    return handle;
}

void VulkanRenderer::DestroyTexture(TextureHandle &handle) {
    if (vk_device_ == VK_NULL_HANDLE) return;

    vkDeviceWaitIdle(vk_device_);

    if (handle.descriptor_set != VK_NULL_HANDLE && vk_texture_pool_ != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(vk_device_, vk_texture_pool_, 1, &handle.descriptor_set);
        handle.descriptor_set = VK_NULL_HANDLE;
    }

    if (handle.image_view != VK_NULL_HANDLE) {
        vkDestroyImageView(vk_device_, handle.image_view, nullptr);
        handle.image_view = VK_NULL_HANDLE;
    }

    if (handle.image != VK_NULL_HANDLE) {
        vkDestroyImage(vk_device_, handle.image, nullptr);
        handle.image = VK_NULL_HANDLE;
    }

    if (handle.memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_device_, handle.memory, nullptr);
        handle.memory = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::SetTexture(TextureHandle &handle) {
    vkCmdBindDescriptorSets(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            main_pipeline_helper_.pipeline_layout, 2, 1,
                            &handle.descriptor_set, 0, VK_NULL_HANDLE);
}

void VulkanRenderer::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkPipelineStageFlags src_stage_flags = 0;
    VkPipelineStageFlags dst_stage_flags = 0;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout ==
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdPipelineBarrier(local_command_buffer, src_stage_flags, dst_stage_flags, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    EndTransientCommandBuffer(local_command_buffer);
}

void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 image_size) {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {static_cast<uint32_t>(image_size.x), static_cast<uint32_t>(image_size.y), 1};

    vkCmdCopyBufferToImage(local_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndTransientCommandBuffer(local_command_buffer);
}

TextureHandle VulkanRenderer::CreateImage(glm::vec2 image_size, VkFormat image_format, VkBufferUsageFlags usage_flags,
                                          VkMemoryPropertyFlags property_flags) {
    TextureHandle handle = {};

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.usage = usage_flags;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = image_size.x;
    image_info.extent.height = image_size.y;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = image_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

    VkResult result = vkCreateImage(vk_device_, &image_info, nullptr, &handle.image);
    if (result != VK_SUCCESS) throw std::runtime_error("failed to create image!");

    VkMemoryRequirements mem_requirements = {};
    vkGetImageMemoryRequirements(vk_device_, handle.image, &mem_requirements);

    std::uint32_t chosen_mem_type = FindMemoryType(mem_requirements.memoryTypeBits, property_flags);

    VkMemoryAllocateInfo allocation_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr, mem_requirements.size, chosen_mem_type
    };

    VkResult alloc_result = vkAllocateMemory(vk_device_, &allocation_info, nullptr, &handle.memory);

    if (alloc_result != VK_SUCCESS) throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(vk_device_, handle.image, handle.memory, 0);

    return handle;
}

void VulkanRenderer::RecreateSwapchain() {
    glm::ivec2 size = window->GetFrameBufferSize();
    while (size.x == 0 || size.y == 0) {
        size = window->GetFrameBufferSize();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vk_device_);
    CleanupSwapchain();

    CreateSwapChain();
    CreateImageViews();
    CreateFramebuffers();
}

void VulkanRenderer::CleanupSwapchain() const {
    if (vk_device_ == VK_NULL_HANDLE) return;

    for (VkFramebuffer framebuffer: vk_swapchain_framebuffers_)
        vkDestroyFramebuffer(vk_device_, framebuffer, nullptr);

    for (VkImageView image_view: vk_swapchain_image_views_)
        vkDestroyImageView(vk_device_, image_view, nullptr);

    if (vk_swapchain_ != VK_NULL_HANDLE) vkDestroySwapchainKHR(vk_device_, vk_swapchain_, nullptr);
}

VulkanRenderer::VulkanRenderer(Window *window): Renderer(window, RendererType::VULKAN) {
#if !defined(NDEBUG)
    validation_ = true;
#endif
    InitializeVulkan();
}

VulkanRenderer::~VulkanRenderer() {
    VulkanRenderer::OnDestroy();
}

bool VulkanRenderer::OnCreate() {
    InitializeVulkan();
    return true;
}

void VulkanRenderer::OnDestroy() {
    DestroyPostProcessingResources();

    if (vk_device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vk_device_);

        // Cleanup skybox resources
        vkDestroyPipeline(vk_device_, skybox_.pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(vk_device_, skybox_.pipeline.pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(vk_device_, skybox_.descriptor_set_layout, nullptr);
        vkDestroyDescriptorPool(vk_device_, skybox_.descriptor_pool, nullptr);
        vkDestroySampler(vk_device_, skybox_.sampler, nullptr);
        vkDestroyImageView(vk_device_, skybox_.view, nullptr);
        vkDestroyImage(vk_device_, skybox_.image, nullptr);
        vkFreeMemory(vk_device_, skybox_.memory, nullptr);
        DestroyBuffer(skybox_.vertex_buffer);
        DestroyBuffer(skybox_.index_buffer);

        CleanupSwapchain();

        DestroyTexture(depth_texture_);

        if (vk_texture_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk_device_, vk_texture_pool_, nullptr);

        if (vk_texture_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_texture_set_layout_, nullptr);

        if (vk_texture_sampler_ != VK_NULL_HANDLE)
            vkDestroySampler(vk_device_, vk_texture_sampler_, nullptr);

        if (vk_uniform_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk_device_, vk_uniform_pool_, nullptr);

        // Unmap memory before destroying buffers
        if (uniform_buffer_location_) {
            vkUnmapMemory(vk_device_, uniform_buffer_.memory);
            uniform_buffer_location_ = nullptr;
        }
        if (bp_buffer_location_) {
            vkUnmapMemory(vk_device_, bp_buffer_handle_.memory);
            bp_buffer_location_ = nullptr;
        }
        if (global_lights_buffer_location_) {
            vkUnmapMemory(vk_device_, g_light_handle_.memory);
            global_lights_buffer_location_ = nullptr;
        }

        DestroyBuffer(g_light_handle_);
        DestroyBuffer(uniform_buffer_);
        DestroyBuffer(bp_buffer_handle_);

        if (vk_uniform_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_uniform_set_layout_, nullptr);

        if (vk_uniform_bp_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_uniform_bp_set_layout_, nullptr);
        if (vk_lights_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_lights_set_layout_, nullptr);

        vkDeviceWaitIdle(vk_device_);

        if (vk_image_available_signal_ != VK_NULL_HANDLE)
            vkDestroySemaphore(vk_device_, vk_image_available_signal_, nullptr);
        if (vk_render_finished_signal_ != VK_NULL_HANDLE)
            vkDestroySemaphore(vk_device_, vk_render_finished_signal_, nullptr);
        if (vk_still_rendering_fence_ != VK_NULL_HANDLE)
            vkDestroyFence(vk_device_, vk_still_rendering_fence_, nullptr);

        if (vk_command_pool_ != VK_NULL_HANDLE)
            vkDestroyCommandPool(vk_device_, vk_command_pool_, nullptr);

        if (main_pipeline_helper_.pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(vk_device_, main_pipeline_helper_.pipeline, nullptr);
        if (main_pipeline_helper_.pipeline_layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(vk_device_, main_pipeline_helper_.pipeline_layout, nullptr);

        if (vk_render_pass_ != VK_NULL_HANDLE)
            vkDestroyRenderPass(vk_device_, vk_render_pass_, nullptr);

        vkDestroyDevice(vk_device_, nullptr);
    }

    if (vk_instance_ != VK_NULL_HANDLE) {
        if (vk_surface_ != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(vk_instance_, vk_surface_, VK_NULL_HANDLE);

        if (vk_debug_messenger_ != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, VK_NULL_HANDLE);

        vkDestroyInstance(vk_instance_, VK_NULL_HANDLE);
    }
}

void VulkanRenderer::Render() {}

void VulkanRenderer::InitializeVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDeviceAndQueues();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayouts();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSignals();
    CreateUniformBuffers();
    CreateDescriptorPools();
    CreateDescriptorSets();
    CreateTextureSampler();
    TransitionImageLayout(depth_texture_.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    CreatePostProcessingResources();
}

void VulkanRenderer::CreateSkyboxResources() {
    spdlog::info("Creating skybox resources");

    // Create descriptor set layout
    CreateSkyboxDescriptorSetLayout();

    // Create skybox pipeline
    CreateSkyboxPipeline();

    // Create skybox geometry
    auto vertices = CreateSkyboxVertices();
    auto indices = CreateSkyboxIndices();

    skybox_.vertex_buffer = CreateVertexBuffer(vertices);
    skybox_.index_buffer = CreateIndexBuffer(indices);

    spdlog::info("Loading skybox textures from CN_Tower directory");

    CreateSkyboxImage(cubemap_);

    spdlog::info("Skybox resources created successfully");
}

void VulkanRenderer::CreateSkyboxDescriptorSetLayout() {
    // Create bindings for uniform buffer and cubemap sampler
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};

    // Uniform buffer binding
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Cubemap sampler binding
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(vk_device_, &layout_info, nullptr, &skybox_.descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create skybox descriptor set layout!");
    }
}

void VulkanRenderer::CreateSkyboxPipeline() {
    skybox_.pipeline = {
        {"shaders/skybox.vert.spv", "shaders/skybox.frag.spv"}, {Skybox::GetBindingDescription()},
        Skybox::GetAttributeDescriptions(), VK_CULL_MODE_NONE, {true, false, VK_COMPARE_OP_LESS_OR_EQUAL},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)}}, {skybox_.descriptor_set_layout}
    };
    CreatePipeline(skybox_.pipeline);
}

std::vector<glm::vec3> VulkanRenderer::CreateSkyboxVertices() {
    std::vector<glm::vec3> vertices = {
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
    };

    spdlog::info("Created skybox vertices: {} vertices", vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i) {
        spdlog::info("Vertex {}: ({}, {}, {})", i,
                     vertices[i].x, vertices[i].y, vertices[i].z);
    }

    return vertices;
}

std::vector<uint32_t> VulkanRenderer::CreateSkyboxIndices() {
    std::vector<uint32_t> indices = {
        0, 1, 3, 3, 1, 2, // front
        4, 5, 0, 0, 5, 1, // left
        3, 2, 7, 7, 2, 6, // right
        4, 0, 7, 7, 0, 3, // top
        1, 5, 2, 2, 5, 6, // bottom
        7, 6, 4, 4, 6, 5 // back
    };

    spdlog::info("Created skybox indices: {} indices ({} triangles)",
                 indices.size(), indices.size() / 3);

    return indices;
}

void VulkanRenderer::CreateSkyboxImage(const std::array<const char *, 6> &cubemap_paths) {
    // Load all 6 faces first to get dimensions and create staging buffer
    int tex_width, tex_height, tex_channels;
    std::vector<stbi_uc *> pixels(6);
    VkDeviceSize face_size = 0;

    // Load all faces and validate dimensions
    for (size_t i = 0; i < 6; i++) {
        pixels[i] = stbi_load(cubemap_paths[i], &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (!pixels[i]) {
            throw std::runtime_error("Failed to load cubemap texture: " + std::string(cubemap_paths[i]));
        }
        if (i == 0) {
            face_size = tex_width * tex_height * 4;
        }
    }

    VkDeviceSize total_size = face_size * 6;

    // Create staging buffer for all faces
    BufferHandle staging_buffer = CreateBuffer(
        total_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    // Copy all faces to staging buffer
    void *data;
    vkMapMemory(vk_device_, staging_buffer.memory, 0, total_size, 0, &data);
    for (size_t i = 0; i < 6; i++) {
        memcpy(static_cast<char *>(data) + (face_size * i), pixels[i], face_size);
        stbi_image_free(pixels[i]);
    }
    vkUnmapMemory(vk_device_, staging_buffer.memory);

    // Create image
    VkImageCreateInfo cubemap_create_info{};
    cubemap_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    cubemap_create_info.imageType = VK_IMAGE_TYPE_2D;
    cubemap_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    cubemap_create_info.extent = {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1};
    cubemap_create_info.mipLevels = 1;
    cubemap_create_info.arrayLayers = 6;
    cubemap_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    cubemap_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    cubemap_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    cubemap_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    cubemap_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cubemap_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (vkCreateImage(vk_device_, &cubemap_create_info, nullptr, &skybox_.image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap image!");
    }

    // Allocate memory
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(vk_device_, skybox_.image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(vk_device_, &alloc_info, nullptr, &skybox_.memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate cubemap image memory!");
    }

    vkBindImageMemory(vk_device_, skybox_.image, skybox_.memory, 0);

    // Transition image layout for copy
    VkCommandBuffer cmd = BeginTransientCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = skybox_.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 6;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    // Copy buffer to image
    std::array<VkBufferImageCopy, 6> copy_regions{};
    for (uint32_t face = 0; face < 6; face++) {
        copy_regions[face].bufferOffset = face * face_size;
        copy_regions[face].bufferRowLength = 0;
        copy_regions[face].bufferImageHeight = 0;
        copy_regions[face].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_regions[face].imageSubresource.mipLevel = 0;
        copy_regions[face].imageSubresource.baseArrayLayer = face;
        copy_regions[face].imageSubresource.layerCount = 1;
        copy_regions[face].imageOffset = {0, 0, 0};
        copy_regions[face].imageExtent = {
            static_cast<uint32_t>(tex_width),
            static_cast<uint32_t>(tex_height),
            1
        };
    }

    vkCmdCopyBufferToImage(cmd,
                           staging_buffer.buffer,
                           skybox_.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           copy_regions.size(),
                           copy_regions.data());

    // Transition to shader read optimal
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    EndTransientCommandBuffer(cmd);

    // Create image view
    VkImageViewCreateInfo cubemap_view_info{};
    cubemap_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    cubemap_view_info.image = skybox_.image;
    cubemap_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    cubemap_view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    cubemap_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    cubemap_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    cubemap_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    cubemap_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    cubemap_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubemap_view_info.subresourceRange.baseMipLevel = 0;
    cubemap_view_info.subresourceRange.levelCount = 1;
    cubemap_view_info.subresourceRange.baseArrayLayer = 0;
    cubemap_view_info.subresourceRange.layerCount = 6;

    if (vkCreateImageView(vk_device_, &cubemap_view_info, nullptr, &skybox_.view) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap image view!");
    }

    // Create sampler
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.mipLodBias = 0.0f;

    if (vkCreateSampler(vk_device_, &sampler_info, nullptr, &skybox_.sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap sampler!");
    }

    // Create descriptor pool for skybox
    std::array pool_sizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 1;

    if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &skybox_.descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create skybox descriptor pool!");
    }

    // Create descriptor set
    VkDescriptorSetAllocateInfo descriptor_alloc_info{};
    descriptor_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_alloc_info.descriptorPool = skybox_.descriptor_pool;
    descriptor_alloc_info.descriptorSetCount = 1;
    descriptor_alloc_info.pSetLayouts = &skybox_.descriptor_set_layout;

    if (vkAllocateDescriptorSets(vk_device_, &descriptor_alloc_info, &skybox_.descriptor_set) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate skybox descriptor set!");
    }

    // Update descriptor set
    std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

    // Uniform buffer descriptor
    VkDescriptorBufferInfo uniform_buffer_info{};
    uniform_buffer_info.buffer = uniform_buffer_.buffer;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformTransformations);

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = skybox_.descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &uniform_buffer_info;

    // Cubemap sampler descriptor
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = skybox_.view;
    image_info.sampler = skybox_.sampler;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = skybox_.descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;

    vkUpdateDescriptorSets(vk_device_, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

    // Cleanup staging buffer
    DestroyBuffer(staging_buffer);
}

void VulkanRenderer::RenderSkybox() {
    static bool first_render = true;
    if (first_render) {
        spdlog::info("First skybox render call");
        first_render = false;
    }

    vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, skybox_.pipeline.pipeline);

    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();
    vkCmdSetViewport(vk_command_buffer_, 0, 1, &viewport);
    vkCmdSetScissor(vk_command_buffer_, 0, 1, &scissor);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(vk_command_buffer_, 0, 1, &skybox_.vertex_buffer.buffer, offsets);
    vkCmdBindIndexBuffer(vk_command_buffer_, skybox_.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // Get the current view-projection matrix but remove translation from view matrix
    UniformTransformations transformations{};
    memcpy(&transformations, uniform_buffer_location_, sizeof(UniformTransformations));

    // Remove translation from view matrix to keep skybox centered on camera
    glm::mat4 view = glm::mat4(glm::mat3(transformations.view)); // Use only rotation part of view matrix

    // Update uniform buffer with skybox matrices
    UniformTransformations skybox_transforms{
        view,
        transformations.projection
    };
    memcpy(uniform_buffer_location_, &skybox_transforms, sizeof(UniformTransformations));

    if (first_render) {
        spdlog::info("Binding skybox descriptor set: {}", (void *) skybox_.descriptor_set);
    }

    vkCmdBindDescriptorSets(vk_command_buffer_,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            skybox_.pipeline.pipeline_layout,
                            0, 1, &skybox_.descriptor_set,
                            0, nullptr);

    if (first_render) {
        spdlog::info("Drawing skybox with {} indices", 36);
    }

    vkCmdDrawIndexed(vk_command_buffer_, 36, 1, 0, 0, 0);

    // Restore original transformations for rest of scene
    memcpy(uniform_buffer_location_, &transformations, sizeof(UniformTransformations));
}

VkFormat VulkanRenderer::FindDepthFormat() const {
    const std::array candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vk_physical_device_, format, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported depth format!");
}

bool VulkanRenderer::HasStencilComponent(VkFormat format) const {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::CreatePostProcessingResources() {
    CreatePostProcessingRenderPass();
    CreatePostProcessingFramebuffer();
    CreatePostProcessingDescriptorSet();
    CreatePostProcessingPipeline();
}

void VulkanRenderer::CreatePostProcessingRenderPass() {
    // Color attachment
    VkAttachmentDescription color_attachment{};
    color_attachment.format = vk_surface_format_.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = FindDepthFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    std::array attachments = {color_attachment, depth_attachment};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_device_, &render_pass_info, nullptr, &post_processing_.render_pass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing render pass!");
    }
}

void VulkanRenderer::CreatePostProcessingFramebuffer() {
    // Create color image
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = vk_extent_.width;
    image_info.extent.height = vk_extent_.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = vk_surface_format_.format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vk_device_, &image_info, nullptr, &post_processing_.color_image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing color image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(vk_device_, post_processing_.color_image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(vk_device_, &alloc_info, nullptr, &post_processing_.color_memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate post-processing image memory!");
    }

    vkBindImageMemory(vk_device_, post_processing_.color_image, post_processing_.color_memory, 0);

    // Create depth image
    image_info.format = FindDepthFormat();
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (vkCreateImage(vk_device_, &image_info, nullptr, &post_processing_.depth_image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing depth image!");
    }

    vkGetImageMemoryRequirements(vk_device_, post_processing_.depth_image, &mem_requirements);

    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(vk_device_, &alloc_info, nullptr, &post_processing_.depth_memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate post-processing depth memory!");
    }

    vkBindImageMemory(vk_device_, post_processing_.depth_image, post_processing_.depth_memory, 0);

    // Create image views
    post_processing_.color_view = CreateImageView(post_processing_.color_image, vk_surface_format_.format,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);
    post_processing_.depth_view = CreateImageView(post_processing_.depth_image, FindDepthFormat(),
                                                  VK_IMAGE_ASPECT_DEPTH_BIT);

    // Create framebuffer
    std::array attachments = {
        post_processing_.color_view,
        post_processing_.depth_view
    };

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = post_processing_.render_pass;
    framebuffer_info.attachmentCount = attachments.size();
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = vk_extent_.width;
    framebuffer_info.height = vk_extent_.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(vk_device_, &framebuffer_info, nullptr, &post_processing_.framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing framebuffer!");
    }
}

void VulkanRenderer::CreatePostProcessingDescriptorSet() {
    // Create descriptor set layout
    VkDescriptorSetLayoutBinding sampler_binding{};
    sampler_binding.binding = 0;
    sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_binding.descriptorCount = 1;
    sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &sampler_binding;

    if (vkCreateDescriptorSetLayout(vk_device_, &layout_info, nullptr, &post_processing_.descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing descriptor set layout!");
    }

    // Create descriptor pool
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = 1;

    if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &post_processing_.descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing descriptor pool!");
    }

    // Create descriptor set
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = post_processing_.descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &post_processing_.descriptor_set_layout;

    if (vkAllocateDescriptorSets(vk_device_, &alloc_info, &post_processing_.descriptor_set) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate post-processing descriptor set!");
    }

    // Create sampler
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (vkCreateSampler(vk_device_, &sampler_info, nullptr, &post_processing_.sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create post-processing sampler!");
    }

    // Update descriptor set
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = post_processing_.color_view;
    image_info.sampler = post_processing_.sampler;

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = post_processing_.descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(vk_device_, 1, &descriptor_write, 0, nullptr);
}

void VulkanRenderer::CreatePostProcessingPipeline() {
    post_processing_.pipeline = {
        {"shaders/post.vert.spv", "shaders/nopost.frag.spv"}, 
        {}, 
        {}, 
        VK_CULL_MODE_BACK_BIT,
        {false, false, VK_COMPARE_OP_ALWAYS},
        {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float)}},  // Push constant for time
        {post_processing_.descriptor_set_layout}
    };
    CreatePipeline(post_processing_.pipeline);
}

void VulkanRenderer::DestroyPostProcessingResources() {
    if (post_processing_.sampler != VK_NULL_HANDLE)
        vkDestroySampler(vk_device_, post_processing_.sampler, nullptr);
    if (post_processing_.descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(vk_device_, post_processing_.descriptor_pool, nullptr);
    if (post_processing_.descriptor_set_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(vk_device_, post_processing_.descriptor_set_layout, nullptr);
    if (post_processing_.pipeline.pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(vk_device_, post_processing_.pipeline.pipeline, nullptr);
    if (post_processing_.pipeline.pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(vk_device_, post_processing_.pipeline.pipeline_layout, nullptr);
    if (post_processing_.framebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(vk_device_, post_processing_.framebuffer, nullptr);
    if (post_processing_.render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(vk_device_, post_processing_.render_pass, nullptr);
    if (post_processing_.color_view != VK_NULL_HANDLE)
        vkDestroyImageView(vk_device_, post_processing_.color_view, nullptr);
    if (post_processing_.color_image != VK_NULL_HANDLE)
        vkDestroyImage(vk_device_, post_processing_.color_image, nullptr);
    if (post_processing_.color_memory != VK_NULL_HANDLE)
        vkFreeMemory(vk_device_, post_processing_.color_memory, nullptr);
    if (post_processing_.depth_view != VK_NULL_HANDLE)
        vkDestroyImageView(vk_device_, post_processing_.depth_view, nullptr);
    if (post_processing_.depth_image != VK_NULL_HANDLE)
        vkDestroyImage(vk_device_, post_processing_.depth_image, nullptr);
    if (post_processing_.depth_memory != VK_NULL_HANDLE)
        vkFreeMemory(vk_device_, post_processing_.depth_memory, nullptr);
}

void VulkanRenderer::ReloadPostProcessingShader(const std::string &fragment_shader_path) {
    // Wait for the device to be idle before destroying the pipeline
    vkDeviceWaitIdle(vk_device_);

    // Destroy only the existing pipeline (not the layout or other resources)
    if (post_processing_.pipeline.pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(vk_device_, post_processing_.pipeline.pipeline, nullptr);
    }

    // Destroy existing pipeline layout if it exists
    if (post_processing_.pipeline.pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vk_device_, post_processing_.pipeline.pipeline_layout, nullptr);
    }

    post_processing_.pipeline = {
        {"shaders/post.vert.spv", fragment_shader_path}, {}, {}, VK_CULL_MODE_BACK_BIT,
        {false, false, VK_COMPARE_OP_ALWAYS},
        {{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4)}},
        {post_processing_.descriptor_set_layout}
    };
    CreatePipeline(post_processing_.pipeline);
}

void VulkanRenderer::HandleShaderSwitch(int key) {
    if (const auto shader = shaders_.find(key); shader == shaders_.end())
        spdlog::warn("Unhandled key press in shader switch: {}", key);
    else
        ReloadPostProcessingShader(shader->second);
}
