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

    return (function != nullptr)
               ? function(instance, pCreateInfo, pAllocator, pDebugMessenger)
               : VK_ERROR_EXTENSION_NOT_PRESENT;
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
    return {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr, 0,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        ValidationCallback, nullptr
    };
}

void VulkanRenderer::SetupDebugMessenger() {
    if (!validation_) { return; }
    VkDebugUtilsMessengerCreateInfoEXT info = GetCreateDebugMessengerInfo();
    const VkResult result = vkCreateDebugUtilsMessengerEXT(vk_instance_, &info, nullptr, &vk_debug_messenger_);
    CheckExcept(result, "Failed to create debug messenger");
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
    std::vector<const char *> res;
    res.reserve(extension_count);
    for (int i = 0; i < extension_count; i++) {
        res.push_back(extension_names[i]);
    }
    return res;
}

std::vector<const char *> VulkanRenderer::GetRequiredInstanceExtensions() const {
    std::vector<const char *> suggested_extensions = GetSuggestedInstanceExtensions();
    std::vector<const char *> required_extensions(suggested_extensions.size());
    std::ranges::copy(suggested_extensions, required_extensions.begin());

#if defined(__APPLE__)
    required_extensions.push_back("VK_KHR_portability_enumeration");
#endif

    if (validation_)
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
    VkApplicationInfo app_info{
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "Vulkan Project",
        VK_MAKE_VERSION(0, 0, 1), "Mixed Engine", VK_MAKE_VERSION(0, 0, 1),
        VK_API_VERSION_1_2
    };

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = GetCreateDebugMessengerInfo();

    VkInstanceCreateInfo create_info{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &app_info, 0,
        nullptr, static_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data()
    };

#if defined(__APPLE__)
    create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (validation_) {
        create_info.pNext = &debug_create_info;
        create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();
    } else {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = nullptr;
    }

    CheckExit(vkCreateInstance(&create_info, nullptr, &vk_instance_), "Failed to create instance!");
}

void VulkanRenderer::CreateSurface() {
    const VkResult res = glfwCreateWindowSurface(vk_instance_, window->getGLFWwindow(), nullptr, &vk_surface_);
    CheckExit(res, "Failed to create window surface!");
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

bool VulkanRenderer::AreAllDeviceExtensionsSupported(VkPhysicalDevice device) {
    return std::ranges::all_of(deviceExtensions,
                               std::bind_front(IsDeviceExtensionWithinList, GetDeviceAvailableExtensions(device)));
}

bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice device) const {
    return FindQueueFamilies(device).isComplete() && AreAllDeviceExtensionsSupported(device) &&
           FindSwapChainSupport(device).IsValid();
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

    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (const auto unique_queue_family: uniqueQueueFamilies)
        queueCreateInfos.push_back({
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
            unique_queue_family, 1, &priority
        });

    VkPhysicalDeviceFeatures required_features{.depthClamp = VK_FALSE, .depthBounds = false};

#ifndef __APPLE__
    required_features.depthBounds = true;
    required_features.depthClamp = true;
#endif
    VkDeviceCreateInfo deviceCreateInfo{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0,
        static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(), 0,
        nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
        &required_features
    };

    VkResult res = vkCreateDevice(vk_physical_device_, &deviceCreateInfo, nullptr, &vk_device_);
    CheckExit(res, "Failed to create logical device!");

    vkGetDeviceQueue(vk_device_, indices.graphicsFamily.value(), 0, &vk_graphics_queue_);
    vkGetDeviceQueue(vk_device_, indices.presentFamily.value(), 0, &vk_present_queue_);
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_R8G8B8A8_SNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    if (auto it = std::ranges::find_if(formats, IsCorrectFormat); it != formats.end())
        return *it;
    return formats[0];
}

VkPresentModeKHR VulkanRenderer::ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> present_modes) {
    return std::ranges::any_of(present_modes, IsMailboxPresent)
               ? VK_PRESENT_MODE_MAILBOX_KHR
               : VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        return capabilities.currentExtent;

    const glm::ivec2 size = window->GetFrameBufferSize();
    VkExtent2D actual_extend = {static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y)};

    actual_extend.width = std::clamp(actual_extend.width, capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actual_extend.height = std::clamp(actual_extend.height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);
    return actual_extend;
}

std::uint32_t VulkanRenderer::ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities) {
    std::uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
        image_count = capabilities.maxImageCount;
    return image_count;
}

void VulkanRenderer::CreateSwapChain() {
    auto [capabilities_khr, formats_khr, present_modes_khr] = FindSwapChainSupport(vk_physical_device_);

    vk_surface_format_ = ChooseSwapchainSurfaceFormat(formats_khr);
    vk_present_mode_ = ChooseSwapchainPresentMode(present_modes_khr);
    vk_extent_ = ChooseSwapchainExtent(capabilities_khr);
    const std::uint32_t image_count = ChooseImageCount(capabilities_khr);

    VkSwapchainCreateInfoKHR create_info_khr{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, vk_surface_, image_count,
        vk_surface_format_.format, vk_surface_format_.colorSpace, vk_extent_, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, capabilities_khr.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, vk_present_mode_, VK_TRUE, VK_NULL_HANDLE
    };

    if (QueueFamilyIndices index = FindQueueFamilies(vk_physical_device_);
        index.graphicsFamily != index.presentFamily) {
        const std::array family_indices = {index.graphicsFamily.value(), index.presentFamily.value()};
        create_info_khr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info_khr.queueFamilyIndexCount = family_indices.size();
        create_info_khr.pQueueFamilyIndices = family_indices.data();
    }

    VkResult res = vkCreateSwapchainKHR(vk_device_, &create_info_khr, nullptr, &vk_swapchain_);
    CheckExit(res, "Failed to create swapchain!");

    std::uint32_t actual_image_count;
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &actual_image_count, nullptr);
    vk_swapchain_images_.resize(actual_image_count);
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &actual_image_count, vk_swapchain_images_.data());
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, const VkFormat format,
                                            VkImageAspectFlags aspect_flags) const {
    VkImageView view;

    VkImageViewCreateInfo view_create_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, image, VK_IMAGE_VIEW_TYPE_2D, format,
        {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        {aspect_flags, 0, 1, 0, 1}
    };

    VkResult res = vkCreateImageView(vk_device_, &view_create_info, nullptr, &view);
    CheckExit(res, "Failed to create image view!");
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

    VkShaderModuleCreateInfo module_info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, buffer.size(),
        reinterpret_cast<const uint32_t *>(buffer.data())
    };

    VkShaderModule vk_shader_module;
    if (VkResult res = vkCreateShaderModule(vk_device_, &module_info, nullptr, &vk_shader_module); res != VK_SUCCESS) {
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
    skybox_.pipeline = {
        {"shaders/skybox.vert.spv", "shaders/skybox.frag.spv"}, {Skybox::GetBindingDescription()},
        Skybox::GetAttributeDescriptions(), VK_CULL_MODE_NONE, {true, false, VK_COMPARE_OP_LESS_OR_EQUAL},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)}}, {skybox_.descriptor_set_layout}
    };
    CreatePipeline(skybox_.pipeline);

    post_processing_.pipeline = {
        {"shaders/post.vert.spv", "shaders/nopost.frag.spv"},
        {},
        {},
        VK_CULL_MODE_BACK_BIT,
        {false, false, VK_COMPARE_OP_ALWAYS},
        {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float)}},
        {post_processing_.descriptor_set_layout}
    };
    CreatePipeline(post_processing_.pipeline);
}

void VulkanRenderer::ReloadPostProcessingShader(const std::string &fragment_shader_path) {
    vkDeviceWaitIdle(vk_device_);

    if (post_processing_.pipeline.pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(vk_device_, post_processing_.pipeline.pipeline, nullptr);

    if (post_processing_.pipeline.pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(vk_device_, post_processing_.pipeline.pipeline_layout, nullptr);

    post_processing_.pipeline = {
        {"shaders/post.vert.spv", fragment_shader_path}, {}, {}, VK_CULL_MODE_BACK_BIT,
        {false, false, VK_COMPARE_OP_ALWAYS},
        {{ VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4)}},
        {post_processing_.descriptor_set_layout}
    };
    CreatePipeline(post_processing_.pipeline);
}

void VulkanRenderer::CreatePipeline(PipelineHelper &pipeline_helper) const {
    VkShaderModule vertex_shader = CreateShaderModule(ReadFile(pipeline_helper.shaders[0]));
    VkShaderModule fragment_shader = CreateShaderModule(ReadFile(pipeline_helper.shaders[1]));

    if (vertex_shader == VK_NULL_HANDLE || fragment_shader == VK_NULL_HANDLE) {
        spdlog::error("Failed finding shaders!");
        std::exit(EXIT_FAILURE);
    }

    std::array stage_infos{
        VkPipelineShaderStageCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertex_shader,
            "main"
        },
        VkPipelineShaderStageCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT,
            fragment_shader, "main"
        }
    };

    std::array dynamic_states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_info{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr,
        0, dynamic_states.size(), dynamic_states.data()
    };

    auto viewport{GetViewport()};
    auto scissor{GetScissor()};

    VkPipelineViewportStateCreateInfo viewport_state_info{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr,
        0, 1, &viewport, 1, &scissor
    };

    auto bindings{pipeline_helper.vertex_input_binding_description};
    auto attributes{pipeline_helper.vertex_input_attribute_description};

    VkPipelineVertexInputStateCreateInfo vertex_input_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr, 0, static_cast<uint32_t>(bindings.size()), bindings.data(),
        static_cast<uint32_t>(attributes.size()), attributes.data()
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_info{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, pipeline_helper.cull_mode,
        VK_FRONT_FACE_CLOCKWISE, VK_FALSE
    };

    rasterization_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_info{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr, 0, pipeline_helper.depth_helper.enable_depth_testing,
        pipeline_helper.depth_helper.enable_depth_writing, pipeline_helper.depth_helper.compare_op,
        VK_FALSE, VK_FALSE
    };

    depthStencil_create_info.minDepthBounds = 0.0f;
    depthStencil_create_info.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr,
        0, VK_FALSE, VK_LOGIC_OP_NO_OP, 1,
        pipeline_helper.color_blend_attachment ? pipeline_helper.color_blend_attachment : &color_blend_attachment
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
        static_cast<uint32_t>(pipeline_helper.descriptor_set_layouts.size()),
        pipeline_helper.descriptor_set_layouts.data(),
        static_cast<uint32_t>(pipeline_helper.push_constant_ranges.size()), pipeline_helper.push_constant_ranges.data()
    };

    VkResult res = vkCreatePipelineLayout(vk_device_, &pipeline_layout_info, nullptr, &pipeline_helper.pipeline_layout);
    CheckExit(res, "failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipeline_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0,
        stage_infos.size(), stage_infos.data(), &vertex_input_info, &input_assembly_info,
        nullptr, &viewport_state_info, &rasterization_state_info, &multisample_info, &depthStencil_create_info,
        &color_blend_state, &dynamic_state_info, pipeline_helper.pipeline_layout, vk_render_pass_, 0
    };

    res = vkCreateGraphicsPipelines(vk_device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                    &pipeline_helper.pipeline);
    CheckExit(res, "failed to create graphics pipeline!");

    vkDestroyShaderModule(vk_device_, vertex_shader, nullptr);
    vkDestroyShaderModule(vk_device_, fragment_shader, nullptr);
}

VkViewport VulkanRenderer::GetViewport() const {
    return {0, 0, static_cast<float>(vk_extent_.width), static_cast<float>(vk_extent_.height), 0.0f, 1.0f};
}

VkRect2D VulkanRenderer::GetScissor() const {
    return {0, 0, vk_extent_};
}

void VulkanRenderer::CreateRenderPass(VkImageLayout layout, VkRenderPass *render_pass) const {
    VkAttachmentDescription color_attachment{
        0, vk_surface_format_.format, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, layout
    };

    VkAttachmentDescription depth_attachment{
        0, FindDepthFormat(), VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    std::array color_attachment_ref = {VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkAttachmentReference depth_attachment_ref{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::array subpass{
        VkSubpassDescription{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = color_attachment_ref.size(),
            .pColorAttachments = color_attachment_ref.data(), .pDepthStencilAttachment = &depth_attachment_ref
        }
    };

    std::array dependency{
        VkSubpassDependency{
            VK_SUBPASS_EXTERNAL, 0,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        }
    };

    std::array attachments{color_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0,
        attachments.size(), attachments.data(), subpass.size(),
        subpass.data(), dependency.size(), dependency.data()
    };

    VkResult res = vkCreateRenderPass(vk_device_, &render_pass_info, nullptr, render_pass);
    CheckExcept(res, "Failed to create render pass!");
}

void VulkanRenderer::CreateRenderPasses() {
    CreateRenderPass(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &vk_render_pass_);
    CreateRenderPass(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &post_processing_.render_pass);
}

void VulkanRenderer::CreateFramebuffers() {
    vk_swapchain_framebuffers_.resize(vk_swapchain_image_views_.size());

    for (std::uint32_t i = 0; i < vk_swapchain_image_views_.size(); i++) {
        std::array attachments = std::array{vk_swapchain_image_views_[i], depth_texture_.image_view};
        VkFramebufferCreateInfo framebuffer_info{
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, vk_render_pass_,
            attachments.size(), attachments.data(), vk_extent_.width, vk_extent_.height, 1
        };

        VkResult res = vkCreateFramebuffer(vk_device_, &framebuffer_info, nullptr, &vk_swapchain_framebuffers_[i]);
        CheckExit(res, "failed to create framebuffer!");
    }
    CreatePostProcessingFramebuffer();
}

void VulkanRenderer::CreatePostProcessingFramebuffer() {
    glm::vec2 size{vk_extent_.width, vk_extent_.height};
    post_processing_.color_handle = CreateImage(size, vk_surface_format_.format,
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    post_processing_.depth_handle = CreateImage(size, FindDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    post_processing_.color_view = CreateImageView(post_processing_.color_handle.image, vk_surface_format_.format,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);
    post_processing_.depth_view = CreateImageView(post_processing_.depth_handle.image, FindDepthFormat(),
                                                  VK_IMAGE_ASPECT_DEPTH_BIT);

    std::array attachments{post_processing_.color_view, post_processing_.depth_view};

    const VkFramebufferCreateInfo framebuffer_info{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, post_processing_.render_pass,
        attachments.size(), attachments.data(), vk_extent_.width, vk_extent_.height, 1
    };

    VkResult res = vkCreateFramebuffer(vk_device_, &framebuffer_info, nullptr, &post_processing_.framebuffer);
    CheckExcept(res, "Failed to create post-processing framebuffer!");
}

void VulkanRenderer::CreateCommandPool() {
    QueueFamilyIndices indices = FindQueueFamilies(vk_physical_device_);
    VkCommandPoolCreateInfo pool_info{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, indices.graphicsFamily.value()
    };

    VkResult res = vkCreateCommandPool(vk_device_, &pool_info, nullptr, &vk_command_pool_);
    CheckExit(res, "failed to create command pool!");
}

void VulkanRenderer::BeginCommands() {
    vkResetCommandBuffer(buffered_frames_[current_frame_].command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    VkResult res = vkBeginCommandBuffer(buffered_frames_[current_frame_].command_buffer, &begin_info);
    CheckExcept(res, "failed to begin command buffer!");

    std::array<VkClearValue, 2> clear_values = {};
    clear_values[0].color = {0.2f, 0.3f, 0.3f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, post_processing_.render_pass,
        post_processing_.framebuffer, {{0, 0}, vk_extent_}, clear_values.size(),
        clear_values.data()
    };

    vkCmdBeginRenderPass(buffered_frames_[current_frame_].command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    RenderSkybox();

    vkCmdBindPipeline(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      main_pipeline_helper_.pipeline);
    VkViewport viewport{GetViewport()};
    VkRect2D scissor{GetScissor()};

    vkCmdSetViewport(buffered_frames_[current_frame_].command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffered_frames_[current_frame_].command_buffer, 0, 1, &scissor);
}

void VulkanRenderer::PostRenderPass() const {
    vkCmdEndRenderPass(buffered_frames_[current_frame_].command_buffer);

    std::array<VkClearValue, 2> clear_values = {};
    clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, vk_render_pass_,
        vk_swapchain_framebuffers_[current_image_index_], {{0, 0}, vk_extent_},
        clear_values.size(), clear_values.data()
    };

    vkCmdBeginRenderPass(buffered_frames_[current_frame_].command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      post_processing_.pipeline.pipeline);
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            post_processing_.pipeline.pipeline_layout, 0, 1,
                            &post_processing_.descriptor_set, 0, nullptr);

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    vkCmdPushConstants(buffered_frames_[current_frame_].command_buffer, post_processing_.pipeline.pipeline_layout,
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(float), &time);

    VkViewport viewport{GetViewport()};
    VkRect2D scissor{GetScissor()};
    vkCmdSetViewport(buffered_frames_[current_frame_].command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffered_frames_[current_frame_].command_buffer, 0, 1, &scissor);

    vkCmdDraw(buffered_frames_[current_frame_].command_buffer, 3, 1, 0, 0);
}

void VulkanRenderer::EndCommands() const {
    PostRenderPass();
    vkCmdEndRenderPass(buffered_frames_[current_frame_].command_buffer);

    const VkResult res = vkEndCommandBuffer(buffered_frames_[current_frame_].command_buffer);
    CheckExcept(res, "failed to end command buffer commands");
}

void VulkanRenderer::CreateSignals() {
    VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
    VkCommandBufferAllocateInfo info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        vk_command_pool_,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    for (int i =0; i < MAX_BUFFERED_FRAMES; i++) {
        VkResult result = vkCreateSemaphore(vk_device_, &semaphore_info, nullptr, &buffered_frames_[i].image_available_semaphore);
        CheckExcept(result, "Failed to create image available semaphore!");

        result = vkCreateSemaphore(vk_device_, &semaphore_info, nullptr, &buffered_frames_[i].render_finished_semaphore);
        CheckExcept(result, "Failed to create render finished semaphore!");

        result = vkCreateFence(vk_device_, &fence_info, nullptr, &buffered_frames_[i].still_rendering_fence);
        CheckExcept(result, "Failed to create fence!");

        result = vkAllocateCommandBuffers(vk_device_, &info, &buffered_frames_[i].command_buffer);
        CheckExcept(result, "Failed to allocate command buffer!");
    }
}

bool VulkanRenderer::BeginFrame() {
    vkWaitForFences(vk_device_, 1, &buffered_frames_[current_frame_].still_rendering_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk_device_, 1, &buffered_frames_[current_frame_].still_rendering_fence);

    VkResult result = vkAcquireNextImageKHR(vk_device_, vk_swapchain_, UINT64_MAX,
                                            buffered_frames_[current_frame_].image_available_semaphore,
                                            VK_NULL_HANDLE,
                                            &current_image_index_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    BeginCommands();
    //SetModelMatrix(glm::mat4(1.0f));
    return true;
}

void VulkanRenderer::EndFrame() {
    EndCommands();

    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1, &buffered_frames_[current_frame_].image_available_semaphore,
        &wait_stage_flags,
        1, &buffered_frames_[current_frame_].command_buffer,
        1, &buffered_frames_[current_frame_].render_finished_semaphore
    };

    VkResult result = vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, buffered_frames_[current_frame_].still_rendering_fence);
    CheckExcept(result, "Failed to submit draw command buffer!");

    VkPresentInfoKHR present_info{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1, &buffered_frames_[current_frame_].render_finished_semaphore,
        1, &vk_swapchain_,
        &current_image_index_
    };

    result = vkQueuePresentKHR(vk_present_queue_, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    current_frame_ = (++current_frame_) % MAX_BUFFERED_FRAMES;
}

std::uint32_t VulkanRenderer::FindMemoryType(const std::uint32_t memory_type_bits,
                                             const VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &memory_properties);
    const std::vector memory_types(memory_properties.memoryTypes,
                                   memory_properties.memoryTypes + memory_properties.memoryTypeCount);

    for (uint32_t i = 0; i < memory_types.size(); i++)
        if (memory_type_bits & (1 << i) && memory_types[i].propertyFlags & properties) return i;

    throw std::runtime_error("failed to find memory type!");
}

BufferHandle VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                          VkMemoryPropertyFlags properties) const {
    BufferHandle buffer_handle{};

    VkBufferCreateInfo buffer_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage,
        VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult res = vkCreateBuffer(vk_device_, &buffer_info, nullptr, &buffer_handle.buffer);
    CheckExcept(res, "failed to create buffer!");

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(vk_device_, buffer_handle.buffer, &memory_requirements);

    std::uint32_t chosen_memory_type = FindMemoryType(memory_requirements.memoryTypeBits, properties);

    VkMemoryAllocateInfo memory_allocate_info{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
        memory_requirements.size, chosen_memory_type
    };

    res = vkAllocateMemory(vk_device_, &memory_allocate_info, nullptr, &buffer_handle.memory);
    CheckExcept(res, "failed to allocate buffer memory!");

    vkBindBufferMemory(vk_device_, buffer_handle.buffer, buffer_handle.memory, 0);

    return buffer_handle;
}

BufferHandle VulkanRenderer::CreateIndexBuffer(const std::vector<uint32_t> &indices) const {
    VkDeviceSize buffer_size = sizeof(uint32_t) * indices.size();
    BufferHandle buffer_handle = CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(vk_device_, buffer_handle.memory, 0, buffer_size, 0, &data);
    std::memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(vk_device_, buffer_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_region{0, 0, buffer_size};
    vkCmdCopyBuffer(transient_commands, buffer_handle.buffer, gpu_handle.buffer, 1, &copy_region);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(buffer_handle);

    return gpu_handle;
}

template<typename T>
BufferHandle VulkanRenderer::CreateVertexBuffer(std::vector<T> vertices) {
    VkDeviceSize buffer_size = sizeof(T) * vertices.size();
    BufferHandle buffer_handle = CreateBuffer(buffer_size,
                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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

template BufferHandle VulkanRenderer::CreateVertexBuffer<oVertex>(std::vector<oVertex> vertices);

void VulkanRenderer::DestroyBuffer(const BufferHandle buffer_handle) const {
    if (vk_device_ == VK_NULL_HANDLE)
        return;

    vkDeviceWaitIdle(vk_device_);

    if (buffer_handle.buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(vk_device_, buffer_handle.buffer, nullptr);
    if (buffer_handle.memory != VK_NULL_HANDLE)
        vkFreeMemory(vk_device_, buffer_handle.memory, nullptr);
}

void VulkanRenderer::RenderModel(const BufferHandle vertex_buffer, const BufferHandle index_buffer,
                                 const std::vector<Mesh> &meshes,
                                 const std::vector<TextureHandle> &textures, std::vector<Material_UBO> material_ubos,
                                 const glm::mat4 &modelMatrix) {
    int offset = 0;
    VkDeviceSize dOffset = 0;
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            main_pipeline_helper_.pipeline_layout,
                            0, 2,
                            std::array{buffered_frames_[current_frame_].uniform_set, vk_bp_set_}.data(), 0, VK_NULL_HANDLE);

    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            main_pipeline_helper_.pipeline_layout,
                            3, 1,
                            std::array{vk_lights_set_}.data(), 0, VK_NULL_HANDLE);
    vkCmdBindVertexBuffers(buffered_frames_[current_frame_].command_buffer, 0, 1, &vertex_buffer.buffer, &dOffset);
    vkCmdBindIndexBuffer(buffered_frames_[current_frame_].command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    SetModelMatrix(modelMatrix);
    for (const auto &[indices, materialId]: meshes) {
        SetTexture(textures[materialId]);
        SetUBO<Material_UBO>(bp_buffer_location_, &material_ubos[materialId]);
        vkCmdDrawIndexed(buffered_frames_[current_frame_].command_buffer, indices.size(), 1, offset, 0, 0);
        offset += static_cast<int>(indices.size());
    }
}

void VulkanRenderer::RenderSkybox() const {
    vkCmdBindPipeline(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skybox_.pipeline.pipeline);

    VkViewport viewport = GetViewport();
    VkRect2D scissor = GetScissor();
    vkCmdSetViewport(buffered_frames_[current_frame_].command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffered_frames_[current_frame_].command_buffer, 0, 1, &scissor);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(buffered_frames_[current_frame_].command_buffer, 0, 1, &skybox_.vertex_buffer.buffer, offsets);
    vkCmdBindIndexBuffer(buffered_frames_[current_frame_].command_buffer, skybox_.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    UniformTransformations transformations{};
    memcpy(&transformations, buffered_frames_[current_frame_].uniform_buffer_location, sizeof(UniformTransformations));

    glm::mat4 view = glm::mat4(glm::mat3(transformations.view));

    UniformTransformations skybox_transforms{view, transformations.projection};
    memcpy(buffered_frames_[current_frame_].uniform_buffer_location, &skybox_transforms, sizeof(UniformTransformations));

    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            skybox_.pipeline.pipeline_layout,
                            0, 1, &skybox_.descriptor_set,
                            0, nullptr);

    vkCmdDrawIndexed(buffered_frames_[current_frame_].command_buffer, 36, 1, 0, 0, 0);

    memcpy(buffered_frames_[current_frame_].uniform_buffer_location, &transformations, sizeof(UniformTransformations));
}

VkFormat VulkanRenderer::FindDepthFormat() const {
    for (const VkFormat format: {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vk_physical_device_, format, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return format;
    }

    throw std::runtime_error("Failed to find supported depth format!");
}

bool VulkanRenderer::HasStencilComponent(const VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::SetModelMatrix(const glm::mat4 &matrix) const {
        vkCmdPushConstants(buffered_frames_[current_frame_].command_buffer, main_pipeline_helper_.pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(glm::mat4), &matrix);
}

void VulkanRenderer::SetViewProjection(const glm::mat4 &matrix, const glm::mat4 &projection,
                                       const glm::vec3 cameraPos) const {
    UniformTransformations transformations{matrix, projection, cameraPos};
    for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
        SetUBO<UniformTransformations>(buffered_frames_[i].uniform_buffer_location, &transformations);
    }
}

template<typename T>
void VulkanRenderer::SetUBO(void *location, T *ubo) {
    memcpy(location, ubo, sizeof(T));
}

template void VulkanRenderer::SetUBO<GlobalLighting>(void *, GlobalLighting *);

template void VulkanRenderer::SetUBO<Material_UBO>(void *, Material_UBO *);

VkCommandBuffer VulkanRenderer::BeginTransientCommandBuffer() const {
    VkCommandBufferAllocateInfo alloc_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, vk_command_pool_,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vk_device_, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void VulkanRenderer::EndTransientCommandBuffer(VkCommandBuffer command_buffer) const {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr,
        nullptr, 1, &command_buffer
    };

    vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk_graphics_queue_);
    vkFreeCommandBuffers(vk_device_, vk_command_pool_, 1, &command_buffer);
}

void VulkanRenderer::CreateUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(UniformTransformations);
    
    for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
        buffered_frames_[i].uniform_buffer_handle = CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(vk_device_, buffered_frames_[i].uniform_buffer_handle.memory, 0, buffer_size, 0, &buffered_frames_[i].uniform_buffer_location);
    }

    VkDeviceSize bp_size = sizeof(Material_UBO);
    bp_buffer_handle_ = CreateBuffer(bp_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(vk_device_, bp_buffer_handle_.memory, 0, bp_size, 0, &bp_buffer_location_);

    VkDeviceSize lights_size = sizeof(GlobalLighting);
    g_light_handle_ = CreateBuffer(lights_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(vk_device_, g_light_handle_.memory, 0, lights_size, 0, &global_lights_buffer_location_);
}

void VulkanRenderer::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                                               VkDescriptorSetLayout *layout) const {
    VkDescriptorSetLayoutCreateInfo uniform_layout_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr, 0, static_cast<uint32_t>(bindings.size()), bindings.data()
    };

    VkResult res = vkCreateDescriptorSetLayout(vk_device_, &uniform_layout_info, nullptr, layout);
    CheckExit(res, "Failed to create uniform descriptor set layout!");
}

void VulkanRenderer::CreateDescriptorSetLayouts() {
    VkDescriptorSetLayoutBinding uniform_layout_binding{
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_ALL_GRAPHICS
    };
    CreateDescriptorSetLayout({uniform_layout_binding}, &vk_uniform_set_layout_);

    VkDescriptorSetLayoutBinding uniform_bp_layout_binding{
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };
    CreateDescriptorSetLayout({uniform_bp_layout_binding}, &vk_uniform_bp_set_layout_);

    VkDescriptorSetLayoutBinding lights_layout_binding{
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };
    CreateDescriptorSetLayout({lights_layout_binding}, &vk_lights_set_layout_);

    VkDescriptorSetLayoutBinding texture_layout_binding{
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };
    CreateDescriptorSetLayout({texture_layout_binding}, &vk_texture_set_layout_);

    const std::vector bindings = {
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
        VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
    };

    CreateDescriptorSetLayout(bindings, &skybox_.descriptor_set_layout);

    VkDescriptorSetLayoutBinding sampler_binding{
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    CreateDescriptorSetLayout({sampler_binding}, &post_processing_.descriptor_set_layout);
}

void VulkanRenderer::CreateDescriptorPools() {
    VkDescriptorPoolSize uniform_pool_sizes{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_BUFFERED_FRAMES + 2};

    VkDescriptorPoolCreateInfo pool_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, MAX_BUFFERED_FRAMES + 2,
        1, &uniform_pool_sizes
    };

    VkResult res = vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &vk_uniform_pool_);
    CheckExit(res, "Failed to create uniform pool!");

    VkDescriptorPoolSize pool_size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};

    VkDescriptorPoolCreateInfo pool_info1{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, 1,
        1, &pool_size
    };

    res = vkCreateDescriptorPool(vk_device_, &pool_info1, nullptr, &post_processing_.descriptor_pool);
    CheckExcept(res, "Failed to create post-processing descriptor pool!");

    VkPhysicalDeviceProperties device_properties{};
    vkGetPhysicalDeviceProperties(vk_physical_device_, &device_properties);

    VkDescriptorPoolSize texture_pool_size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024};

    VkDescriptorPoolCreateInfo texture_pool_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 1024, 1, &texture_pool_size
    };

    res = vkCreateDescriptorPool(vk_device_, &texture_pool_info, nullptr, &vk_texture_pool_);
    CheckExit(res, "Failed to create texture pool!");
}

void VulkanRenderer::AllocateDescriptorSet(const VkDescriptorSetAllocateInfo &alloc_info,
                                           VkDescriptorSet *layout) const {
    const VkResult res = vkAllocateDescriptorSets(vk_device_, &alloc_info, layout);
    CheckExit(res, "Failed to allocate descriptor sets!");
}

void VulkanRenderer::CreateDescriptorSets() {
    for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
        VkDescriptorSetAllocateInfo alloc_info{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_uniform_set_layout_
        };
        AllocateDescriptorSet(alloc_info, &buffered_frames_[i].uniform_set);
        
        VkDescriptorBufferInfo info{buffered_frames_[i].uniform_buffer_handle.buffer, 0, sizeof(UniformTransformations)};
        VkWriteDescriptorSet uniform_write{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, buffered_frames_[i].uniform_set, 0, 0, 1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &info
        };
        vkUpdateDescriptorSets(vk_device_, 1, &uniform_write, 0, nullptr);
    }

    VkDescriptorSetAllocateInfo bp_descriptor_set_allocate_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_uniform_bp_set_layout_
    };
    AllocateDescriptorSet(bp_descriptor_set_allocate_info, &vk_bp_set_);
    VkDescriptorBufferInfo blin{bp_buffer_handle_.buffer, 0, sizeof(Material_UBO)};

    VkDescriptorSetAllocateInfo lights_descriptor_set_allocate_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_uniform_pool_, 1, &vk_lights_set_layout_
    };
    AllocateDescriptorSet(lights_descriptor_set_allocate_info, &vk_lights_set_);
    VkDescriptorBufferInfo light{g_light_handle_.buffer, 0, sizeof(GlobalLighting)};

    VkDescriptorSetAllocateInfo alloc_info1{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, vk_texture_pool_,
        1, &post_processing_.descriptor_set_layout
    };
    AllocateDescriptorSet(alloc_info1, &post_processing_.descriptor_set);

    VkDescriptorImageInfo image_info{
        post_processing_.sampler, post_processing_.color_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::array writes = {
        VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, vk_bp_set_, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            nullptr, &blin
        },
        VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, vk_lights_set_, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            nullptr, &light
        },
        VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, post_processing_.descriptor_set, 0, 0,
            1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &image_info,
        }
    };

    vkUpdateDescriptorSets(vk_device_, writes.size(), writes.data(), 0, nullptr);
}

void VulkanRenderer::CreateTextureSampler() {
    VkSamplerCreateInfo sampler_info{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, VK_FALSE, 1.0f, VK_FALSE,
        VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE
    };

    VkResult res = vkCreateSampler(vk_device_, &sampler_info, nullptr, &vk_texture_sampler_);
    CheckExit(res, "Failed to create texture sampler!");

    sampler_info = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    };

    res = vkCreateSampler(vk_device_, &sampler_info, nullptr, &post_processing_.sampler);
    CheckExcept(res, "Failed to create post-processing sampler!");
}

void VulkanRenderer::CreateDepthResources() {
    VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    depth_texture_ = CreateImage({vk_extent_.width, vk_extent_.height}, depth_format,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depth_texture_.image_view = CreateImageView(depth_texture_.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRenderer::CreateSkyboxImage(const std::array<std::string, 6> &cubemap_paths) {
    int tex_width, tex_height, tex_channels;
    std::vector<stbi_uc *> pixels(6);
    VkDeviceSize face_size = 0;

    for (size_t i = 0; i < 6; i++) {
        pixels[i] = stbi_load(cubemap_paths[i].c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (!pixels[i])
            throw std::runtime_error("Failed to load cubemap texture: " + std::string(cubemap_paths[i]));
        if (i == 0)
            face_size = tex_width * tex_height * 4;
    }

    VkDeviceSize total_size{face_size * 6};

    BufferHandle staging_buffer = CreateBuffer(total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(vk_device_, staging_buffer.memory, 0, total_size, 0, &data);
    for (size_t i = 0; i < 6; i++) {
        memcpy(static_cast<char *>(data) + (face_size * i), pixels[i], face_size);
        stbi_image_free(pixels[i]);
    }
    vkUnmapMemory(vk_device_, staging_buffer.memory);

    VkImageCreateInfo cubemap_create_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, VK_NULL_HANDLE, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
        {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1}, 1,
        6, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkResult res = vkCreateImage(vk_device_, &cubemap_create_info, nullptr, &skybox_.image);
    CheckExcept(res, "Failed to create cubemap image!");

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(vk_device_, skybox_.image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, mem_requirements.size,
        FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    res = vkAllocateMemory(vk_device_, &alloc_info, nullptr, &skybox_.memory);
    CheckExcept(res, "Failed to allocate cubemap image memory!");

    vkBindImageMemory(vk_device_, skybox_.image, skybox_.memory, 0);

    VkCommandBuffer cmd = BeginTransientCommandBuffer();

    VkImageMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, skybox_.image, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    std::array<VkBufferImageCopy, 6> copy_regions{};
    for (uint32_t face = 0; face < 6; face++)
        copy_regions[face] = {
            face * face_size, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, face, 1},
            {0, 0, 0}, {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1}
        };

    vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, skybox_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           copy_regions.size(), copy_regions.data());

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    EndTransientCommandBuffer(cmd);

    VkComponentMapping map{
        VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
    };

    VkImageSubresourceRange sub{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6};

    VkImageViewCreateInfo cubemap_view_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
        skybox_.image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_SRGB, map, sub
    };

    res = vkCreateImageView(vk_device_, &cubemap_view_info, nullptr, &skybox_.view);
    CheckExcept(res, "Failed to create cubemap image view!");

    VkSamplerCreateInfo sampler_info{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, VK_FALSE, 1.0f,
        VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE
    };

    res = vkCreateSampler(vk_device_, &sampler_info, nullptr, &skybox_.sampler);
    CheckExcept(res, "Failed to create cubemap sampler!");

    std::array pool_sizes{
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
    };

    VkDescriptorPoolCreateInfo pool_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, 1,
        pool_sizes.size(), pool_sizes.data()
    };

    vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &skybox_.descriptor_pool);
    CheckExcept(res, "Failed to create skybox descriptor pool!");

    VkDescriptorSetAllocateInfo descriptor_alloc_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
        skybox_.descriptor_pool, 1, &skybox_.descriptor_set_layout
    };

    vkAllocateDescriptorSets(vk_device_, &descriptor_alloc_info, &skybox_.descriptor_set);
    CheckExcept(res, "Failed to allocate skybox descriptor set!");

    VkDescriptorBufferInfo uniform_buffer_info{buffered_frames_[current_frame_].uniform_buffer_handle.buffer, 0, sizeof(UniformTransformations)};
    VkDescriptorImageInfo image_info{skybox_.sampler, skybox_.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    std::array descriptor_writes{
        VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, skybox_.descriptor_set, 0,
            0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &uniform_buffer_info
        },
        VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, skybox_.descriptor_set, 1,
            0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &image_info
        }
    };

    vkUpdateDescriptorSets(vk_device_, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

    DestroyBuffer(staging_buffer);
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

    VkDescriptorSetAllocateInfo descriptor_set_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
        vk_texture_pool_, 1, &vk_texture_set_layout_
    };
    AllocateDescriptorSet(descriptor_set_info, &handle.descriptor_set);

    handle.image_view = CreateImageView(handle.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    VkDescriptorImageInfo image_info{vk_texture_sampler_, handle.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet descriptor_write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, handle.descriptor_set,
        0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &image_info
    };

    vkUpdateDescriptorSets(vk_device_, 1, &descriptor_write, 0, nullptr);

    DestroyBuffer(staging_buffer);
    return handle;
}

void VulkanRenderer::SetTexture(const TextureHandle &handle) const {
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            main_pipeline_helper_.pipeline_layout,
                            2, 1, &handle.descriptor_set, 0, VK_NULL_HANDLE);
}

void VulkanRenderer::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .oldLayout = oldLayout, .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .image = image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

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

void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 image_size) const {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkBufferImageCopy region{
        0, 0, 0,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0},
        {static_cast<uint32_t>(image_size.x), static_cast<uint32_t>(image_size.y), 1}
    };

    vkCmdCopyBufferToImage(local_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndTransientCommandBuffer(local_command_buffer);
}

TextureHandle VulkanRenderer::CreateImage(glm::vec2 image_size, VkFormat image_format, VkBufferUsageFlags usage_flags,
                                          VkMemoryPropertyFlags property_flags) const {
    TextureHandle handle{};

    VkImageCreateInfo image_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D, image_format,
        static_cast<uint32_t>(image_size.x), static_cast<uint32_t>(image_size.y), 1, 1, 1, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL, usage_flags, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkResult res = vkCreateImage(vk_device_, &image_info, nullptr, &handle.image);
    CheckExcept(res, "failed to create image!");

    VkMemoryRequirements mem_requirements = {};
    vkGetImageMemoryRequirements(vk_device_, handle.image, &mem_requirements);

    std::uint32_t chosen_mem_type = FindMemoryType(mem_requirements.memoryTypeBits, property_flags);

    VkMemoryAllocateInfo allocation_info{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, mem_requirements.size,
        chosen_mem_type
    };

    res = vkAllocateMemory(vk_device_, &allocation_info, nullptr, &handle.memory);
    CheckExcept(res, "failed to allocate image memory!");

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

    for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
        vkDestroySemaphore(vk_device_, buffered_frames_[i].image_available_semaphore, nullptr);
        vkDestroySemaphore(vk_device_, buffered_frames_[i].render_finished_semaphore, nullptr);
        vkDestroyFence(vk_device_, buffered_frames_[i].still_rendering_fence, nullptr);
        vkFreeCommandBuffers(vk_device_, vk_command_pool_, 1, &buffered_frames_[i].command_buffer);
    }
    CreateSignals();
    
    current_frame_ = 0;
}

void VulkanRenderer::CreateSkyboxResources() {
    skybox_.vertex_buffer = CreateVertexBuffer(CreateSkyboxVertices());
    skybox_.index_buffer = CreateIndexBuffer(CreateSkyboxIndices());
    CreateSkyboxImage(cubemap_);
}

void VulkanRenderer::HandleShaderSwitch(int key) {
    if (const auto shader = shaders_.find(key); shader == shaders_.end())
        spdlog::warn("Unhandled key press in shader switch: {}", key);
    else
        ReloadPostProcessingShader(shader->second);
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

void VulkanRenderer::DestroyTexture(TextureHandle &handle) const {
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

void VulkanRenderer::CleanupSwapchain() const {
    if (vk_device_ == VK_NULL_HANDLE) return;

    for (VkFramebuffer framebuffer: vk_swapchain_framebuffers_)
        vkDestroyFramebuffer(vk_device_, framebuffer, nullptr);

    for (VkImageView image_view: vk_swapchain_image_views_)
        vkDestroyImageView(vk_device_, image_view, nullptr);

    if (vk_swapchain_ != VK_NULL_HANDLE) vkDestroySwapchainKHR(vk_device_, vk_swapchain_, nullptr);
}

void VulkanRenderer::DestroyPostProcessingResources() const {
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
    if (post_processing_.depth_view != VK_NULL_HANDLE)
        vkDestroyImageView(vk_device_, post_processing_.depth_view, nullptr);
}

void VulkanRenderer::OnDestroy() {
    DestroyPostProcessingResources();

    if (vk_device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vk_device_);

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
        DestroyTexture(post_processing_.color_handle);
        DestroyTexture(post_processing_.depth_handle);

        if (vk_texture_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk_device_, vk_texture_pool_, nullptr);
        if (vk_texture_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_texture_set_layout_, nullptr);
        if (vk_texture_sampler_ != VK_NULL_HANDLE)
            vkDestroySampler(vk_device_, vk_texture_sampler_, nullptr);
        if (vk_uniform_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk_device_, vk_uniform_pool_, nullptr);

        for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
            if (buffered_frames_[i].uniform_buffer_location) {
                vkUnmapMemory(vk_device_, buffered_frames_[i].uniform_buffer_handle.memory);
                buffered_frames_[i].uniform_buffer_location = nullptr;
            }
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
        for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
            DestroyBuffer(buffered_frames_[i].uniform_buffer_handle);
        }
        DestroyBuffer(bp_buffer_handle_);

        if (vk_uniform_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_uniform_set_layout_, nullptr);
        if (vk_uniform_bp_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_uniform_bp_set_layout_, nullptr);
        if (vk_lights_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(vk_device_, vk_lights_set_layout_, nullptr);

        vkDeviceWaitIdle(vk_device_);

        for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i) {
            vkDestroySemaphore(vk_device_, buffered_frames_[i].image_available_semaphore, nullptr);
            vkDestroySemaphore(vk_device_, buffered_frames_[i].render_finished_semaphore, nullptr);
            vkDestroyFence(vk_device_, buffered_frames_[i].still_rendering_fence, nullptr);
        }

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
    CreateRenderPasses();
    CreateDescriptorSetLayouts();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandPool();
    CreateSignals();
    CreateUniformBuffers();
    CreateDescriptorPools();
    CreateTextureSampler();
    CreateDescriptorSets();
    TransitionImageLayout(depth_texture_.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
