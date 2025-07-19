//
// Created by andre on 6/05/2025.
//

#pragma once

#include <BufferHandle.h>
#include <GlobalLight.h>
#include <TextureHandle.h>
#include <render/Renderer.h>
#include <window/Window.h>

#define MAX_BUFFERED_FRAMES (4)

struct oVertex;
struct Material_UBO;
struct Mesh;
class ObjectComponent;
const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
    "VK_KHR_portability_subset"
#endif
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
    std::vector<std::string> shaders{};
    std::vector<VkVertexInputBindingDescription> vertex_input_binding_description{};
    std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_description{};
    VkCullModeFlags cull_mode{};
    DepthHelper depth_helper{};
    std::vector<VkPushConstantRange> push_constant_ranges{};
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineColorBlendAttachmentState *color_blend_attachment = VK_NULL_HANDLE;
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
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkImageView color_view = VK_NULL_HANDLE;
    VkImageView depth_view = VK_NULL_HANDLE;
    TextureHandle depth_handle{};
    TextureHandle color_handle{};
    PipelineHelper pipeline;
    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};

struct Frame {
    VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
    VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
    VkFence still_rendering_fence = VK_NULL_HANDLE;

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    VkDescriptorSet uniform_set = VK_NULL_HANDLE;
    BufferHandle uniform_buffer_handle;
    void *uniform_buffer_location = VK_NULL_HANDLE;
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
                     const std::vector<TextureHandle> &textures, std::vector<Material_UBO> material_ubos,
                     const glm::mat4 &modelMatrix);

    bool BeginFrame();

    void EndFrame();

    [[nodiscard]] BufferHandle CreateIndexBuffer(const std::vector<uint32_t> &indices) const;

    template<typename T>
    BufferHandle CreateVertexBuffer(std::vector<T> vertices);

    TextureHandle CreateTexture(const char *path);

    void SetViewProjection(const glm::mat4 &matrix, const glm::mat4 &projection, glm::vec3 cameraPos) const;

    template<typename T>
    static void SetUBO(void *location, T *ubo);

    void DestroyTexture(TextureHandle &handle) const;

    void DestroyBuffer(BufferHandle buffer_handle) const;

    [[nodiscard]] glm::ivec2 GetWindowSize() const { return window->GetFrameBufferSize(); }

    void ReloadPostProcessingShader(const std::string &fragment_shader_path);

    void HandleShaderSwitch(int key);

    std::array<const char *, 6> cubemap_{};

    void CreateSkyboxResources();

    void *global_lights_buffer_location_ = VK_NULL_HANDLE;

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

    void CreatePipeline(PipelineHelper &pipeline_helper) const;

    [[nodiscard]] VkViewport GetViewport() const;

    [[nodiscard]] VkRect2D GetScissor() const;

    void CreateRenderPass(VkImageLayout layout, VkRenderPass *render_pass) const;

    void CreateRenderPasses();

    void CreateFramebuffers();

    void CreateCommandPool();

    void BeginCommands();

    void PostRenderPass() const;

    void EndCommands() const;

    void CreateSignals();

    [[nodiscard]] std::uint32_t FindMemoryType(std::uint32_t memory_type_bits, VkMemoryPropertyFlags properties) const;

    [[nodiscard]] BufferHandle CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;

    void SetModelMatrix(const glm::mat4 &matrix) const;

    [[nodiscard]] VkCommandBuffer BeginTransientCommandBuffer() const;

    void EndTransientCommandBuffer(VkCommandBuffer command_buffer) const;

    void CreateUniformBuffers();

    void CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                                   VkDescriptorSetLayout *layout) const;

    void CreateDescriptorSetLayouts();

    void CreateDescriptorPools();

    void AllocateDescriptorSet(const VkDescriptorSetAllocateInfo &alloc_info, VkDescriptorSet *layout) const;

    void CreateDescriptorSets();

    void CreateTextureSampler();

    void CreateDepthResources();

    void SetTexture(const TextureHandle &handle) const;

    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;

    void CopyBufferToImage(VkBuffer buffer, VkImage image, glm::vec2 image_size) const;

    [[nodiscard]] TextureHandle CreateImage(glm::vec2 image_size, VkFormat image_format, VkBufferUsageFlags usage_flags,
                              VkMemoryPropertyFlags property_flags) const;

    void RecreateSwapchain();

    void CleanupSwapchain() const;

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
    std::vector<VkImage> vk_swapchain_images_{};
    std::vector<VkImageView> vk_swapchain_image_views_{};
    std::vector<VkFramebuffer> vk_swapchain_framebuffers_{};

    PipelineHelper main_pipeline_helper_{};

    VkRenderPass vk_render_pass_ = VK_NULL_HANDLE;

    VkCommandPool vk_command_pool_ = VK_NULL_HANDLE;

    VkFence vk_still_rendering_fence_ = VK_NULL_HANDLE;

    std::uint32_t current_image_index_ = 0;

    VkDescriptorSetLayout vk_uniform_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool vk_uniform_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_uniform_set_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout vk_texture_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool vk_texture_pool_ = VK_NULL_HANDLE;
    VkSampler vk_texture_sampler_ = VK_NULL_HANDLE;
    TextureHandle depth_texture_{};

    std::array<Frame, MAX_BUFFERED_FRAMES> buffered_frames_;
    std::int32_t current_frame_ = 0;

    VkDescriptorSetLayout vk_uniform_bp_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_bp_set_ = VK_NULL_HANDLE;
    BufferHandle bp_buffer_handle_{};
    void *bp_buffer_location_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout vk_lights_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet vk_lights_set_ = VK_NULL_HANDLE;
    BufferHandle g_light_handle_{};

    Skybox skybox_{};

    void CreateSkyboxImage(const std::array<const char *, 6> &cubemap_paths);

    void RenderSkybox() const;

    [[nodiscard]] VkFormat FindDepthFormat() const;

    [[nodiscard]] static bool HasStencilComponent(VkFormat format);

    void CreatePostProcessingFramebuffer();

    void DestroyPostProcessingResources() const;

    PostProcessing post_processing_{};
};
