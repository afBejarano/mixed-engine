//
// Created by andre on 6/05/2025.
//

#pragma once

#include <BufferHandle.h>
#include <GlobalLight.h>
#include <TextureHandle.h>
#include <Vertex.h>
#include <render/Renderer.h>
#include <window/Window.h>

struct Mesh;
struct oVertex;
struct Material_UBO;
class ObjectComponent;
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

struct DepthHelper {
    bool enable_depth_testing = false;
    bool enable_depth_writing = false;
    VkCompareOp compare_op;
};

struct PipelineHelper {
    std::vector<std::string> shaders;
    std::vector<VkVertexInputBindingDescription> vertex_input_binding_description{};
    std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_description{};
    VkCullModeFlags cull_mode{};
    DepthHelper depth_helper{};
    std::vector<VkPushConstantRange> push_constant_ranges{};
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkPipelineColorBlendAttachmentState *color_blend_attachment = nullptr;
};

struct Skybox {
    VkImage image{};
    VkDeviceMemory memory{};
    VkImageView view{};
    VkSampler sampler{};
    VkDescriptorSet descriptor_set{};
    VkDescriptorSetLayout descriptor_set_layout{};
    PipelineHelper pipeline;
    VkDescriptorPool descriptor_pool{};
    BufferHandle vertex_buffer{};
    BufferHandle index_buffer{};

    static VkVertexInputBindingDescription GetBindingDescription() {
        return VkVertexInputBindingDescription{0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}}; // position only
    }
};

struct PostProcessing {
    VkRenderPass render_pass;
    VkFramebuffer framebuffer;
    VkImage color_image;
    VkDeviceMemory color_memory;
    VkImageView color_view;
    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_view;
    PipelineHelper pipeline;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    VkSampler sampler;
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

    void RenderModel(BufferHandle vertex_buffer, BufferHandle index_buffer, const std::vector<Mesh> &meshes,
                     std::vector<TextureHandle> &textures, std::vector<Material_UBO> material_ubos,
                     const glm::mat4 &modelMatrix);

    bool BeginFrame();
    void EndFrame();

    BufferHandle CreateIndexBuffer(std::vector<uint32_t> indices);
    BufferHandle CreateVertexBuffer(std::vector<oVertex> vertices);
    BufferHandle CreateVertexBuffer(const std::vector<glm::vec3> &vertices);
    TextureHandle CreateTexture(const char *path);
    void SetViewProjection(glm::mat4 matrix, glm::mat4 projection, glm::vec3 cameraPos);

    void DestroyTexture(TextureHandle &handle);
    void DestroyBuffer(BufferHandle buffer_handle) const;
    void SetLightsUBO(GlobalLighting *global_lighting);

    glm::ivec2 GetWindowSize() {
        return window->GetFrameBufferSize();
    }

    void ReloadPostProcessingShader(const std::string &fragment_shader_path);
    void HandleShaderSwitch(int key);
    std::unordered_map<int, std::string> shaders_ = {};
private:
    void PickPhysicalDevice();
    void CreateLogicalDeviceAndQueues();
    static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats);
    static VkPresentModeKHR ChooseSwapchainPresentMode(std::vector<VkPresentModeKHR> present_modes);
    [[nodiscard]] VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    static std::uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);
    void CreateSwapChain();
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) const;
    void CreateImageViews();
    [[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<std::uint8_t> &buffer) const;
    void CreateGraphicsPipeline();
    void CreatePipeline(PipelineHelper &pipeline_helper);
    [[nodiscard]] VkViewport GetViewport() const;
    [[nodiscard]] VkRect2D GetScissor() const;
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void BeginCommands();
    void BeginCommands() const;
    void EndCommands() const;
    void CreateSignals();
    [[nodiscard]] std::uint32_t FindMemoryType(std::uint32_t memory_type_bits, VkMemoryPropertyFlags properties) const;
    BufferHandle CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void RenderBuffer(BufferHandle buffer_handle, std::uint32_t vertex_count);
    void RenderIndexedBuffer(BufferHandle vertex_buffer_handle, BufferHandle index_buffer_handle,
                             std::uint32_t index_count, std::int32_t index_offset);

    void SetModelMatrix(const glm::mat4 &matrix) const;
    void SetUbo(Material_UBO &material_ubos) const;
    VkCommandBuffer BeginTransientCommandBuffer();
    void EndTransientCommandBuffer(VkCommandBuffer command_buffer);
    void CreateUniformBuffers();
    void CreateDescriptorSetLayouts();
    void CreateDescriptorPools();
    void CreateDescriptorSets();
    void CreateTextureSampler();
    void CreateDepthResources();
    void SetTexture(TextureHandle &handle);
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 image_size);
    TextureHandle CreateImage(glm::vec2 image_size, VkFormat image_format, VkBufferUsageFlags usage_flags,
                              VkMemoryPropertyFlags property_flags);
    void RecreateSwapchain();
    void CleanupSwapchain() const;
    [[nodiscard]] std::vector<VkPhysicalDevice> GetPhysicalDevices() const;
    void SetUpData();
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
    void SetGlobalLights(GlobalLighting *global);

    bool validation_ = false;

    VkInstance vk_instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT vk_debug_messenger_ = VK_NULL_HANDLE;

    VkPhysicalDevice vk_physical_device_ = VK_NULL_HANDLE;
    VkDevice vk_device_ = VK_NULL_HANDLE;
    VkQueue vk_graphics_queue_ = VK_NULL_HANDLE;
    VkQueue vk_present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR vk_surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR vk_swapchain_ = VK_NULL_HANDLE;
    VkSurfaceFormatKHR vk_surface_format_{};
    VkPresentModeKHR vk_present_mode_{};
    VkExtent2D vk_extent_{};
    std::vector<VkImage> vk_swapchain_images_;
    std::vector<VkImageView> vk_swapchain_image_views_;
    std::vector<VkFramebuffer> vk_swapchain_framebuffers_;

    PipelineHelper main_pipeline_helper_;

    VkRenderPass vk_render_pass_ = VK_NULL_HANDLE;

    VkCommandPool vk_command_pool_ = VK_NULL_HANDLE;
    VkCommandBuffer vk_command_buffer_ = VK_NULL_HANDLE;

    VkSemaphore vk_image_available_signal_ = VK_NULL_HANDLE;
    VkSemaphore vk_render_finished_signal_ = VK_NULL_HANDLE;
    VkFence vk_still_rendering_fence_ = VK_NULL_HANDLE;

    std::uint32_t current_image_index_ = 0;

    VkDescriptorSetLayout vk_uniform_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool vk_uniform_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_uniform_set_ = VK_NULL_HANDLE;
    BufferHandle uniform_buffer_;
    void *uniform_buffer_location_;

    VkDescriptorSetLayout vk_texture_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool vk_texture_pool_ = VK_NULL_HANDLE;
    VkSampler vk_texture_sampler_ = VK_NULL_HANDLE;
    TextureHandle depth_texture_;

    VkDescriptorSetLayout vk_uniform_bp_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_bp_set_ = VK_NULL_HANDLE;
    BufferHandle bp_buffer_handle_;
    void *bp_buffer_location_;

    std::vector<Vertex> vertices = {
        Vertex{glm::vec3{0.0f, -0.5f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}},
        Vertex{glm::vec3{-0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}}
    };

    VkDescriptorSetLayout vk_lights_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_lights_set_ = VK_NULL_HANDLE;
    BufferHandle g_light_handle_;
    void *global_lights_buffer_location_ = nullptr;

    Skybox skybox_{};

    void CreateSkyboxResources();
    void CreateSkyboxPipeline();
    void CreateSkyboxDescriptorSetLayout();
    void CreateSkyboxImage(const std::array<const char *, 6> &cubemap_paths);
    void RenderSkybox();
    std::vector<glm::vec3> CreateSkyboxVertices();
    std::vector<uint32_t> CreateSkyboxIndices();

    VkFormat FindDepthFormat() const;
    bool HasStencilComponent(VkFormat format) const;

    void CreatePostProcessingResources();
    void CreatePostProcessingPipeline();
    void CreatePostProcessingRenderPass();
    void CreatePostProcessingFramebuffer();
    void CreatePostProcessingDescriptorSet();
    void DestroyPostProcessingResources();

    PostProcessing post_processing_{};
};
