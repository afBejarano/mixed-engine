//
// Created by Yibuz Pokopodrozo on 2025-05-16.
//

#pragma once

struct TextureHandle {
    VkImage image = VK_NULL_HANDLE;
    VkImageView image_view = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};