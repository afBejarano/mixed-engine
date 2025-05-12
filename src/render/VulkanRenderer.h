//
// Created by andre on 6/05/2025.
//

#pragma once

#include "Renderer.h"
#include "window/Window.h"

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapchainSupportCapabilities {
    VkSurfaceCapabilitiesKHR capabilities_khr{};
    std::vector<VkSurfaceFormatKHR> formats_khr{};
    std::vector<VkPresentModeKHR> present_modes_khr{};

    [[nodiscard]] bool IsValid() const {
        return !formats_khr.empty() && !present_modes_khr.empty();
    }
};

class VulkanRenderer : public Renderer {
public:
    explicit VulkanRenderer(Window *window);
    ~VulkanRenderer() override;

    VulkanRenderer(const VulkanRenderer &) = delete; /// Copy constructor
    VulkanRenderer(VulkanRenderer &&) = delete; /// Move constructor
    VulkanRenderer &operator=(const VulkanRenderer &) = delete; /// Copy operator
    VulkanRenderer &operator=(VulkanRenderer &&) = delete; /// Move operator

    bool OnCreate() override;
    void OnDestroy() override;
    void Render() override;

private:
    void PickPhysicalDevice();
    void CreateLogicalDeviceAndQueues();
    static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats);
    static VkPresentModeKHR ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> present_modes);
    VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    static std::uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);
    [[nodiscard]] std::vector<VkPhysicalDevice> GetPhysicalDevices() const;
    void InitializeVulkan();
    void SetupDebugMessenger();
    static bool AreAllLayersSupported(const std::vector<const char *> &extensions);
    void CreateInstance();
    void CreateSurface();
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    SwapchainSupportCapabilities FindSwapChainSupport(VkPhysicalDevice device) const;
    static std::vector<VkExtensionProperties> GetDeviceAvailableExtensions(VkPhysicalDevice device);
    static bool AreAllDeviceExtensionsSupported(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device) const;
    static bool AreAllExtensionsSupported(const std::vector<const char *> &extensions);
    [[nodiscard]] std::vector<const char *> GetRequiredInstanceExtensions() const;
    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static std::vector<const char *> GetSuggestedInstanceExtensions();

    bool validation_ = false;
    VkInstance vk_instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR vk_surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice vk_physical_device_ = VK_NULL_HANDLE;
    VkDevice vk_device_ = VK_NULL_HANDLE;
    VkQueue vk_graphics_queue_ = VK_NULL_HANDLE;
    VkQueue vk_present_queue_ = VK_NULL_HANDLE;

};
