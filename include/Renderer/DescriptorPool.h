#pragma once

#include "Device.h"
#include "DescriptorSetLayout.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Shooter {
    namespace Renderer {
        struct DescriptorPoolSizeInfo {
            std::shared_ptr<VulkanDevice>& device;
            std::vector<VkDescriptorPoolSize> sets;
            VkDescriptorPoolCreateFlags flags;
            uint32_t max_sets;
        };

        struct DescriptorBufferAllocateInfo {
            const std::vector<VkDescriptorSetLayout> layouts;
            const std::vector<VkDescriptorBufferInfo> buffer_info;
            uint32_t binding;
            VkDescriptorType descriptor_type;
        };

        class DescriptorPool {
            public:
                DescriptorPool( DescriptorPoolSizeInfo& );
                ~DescriptorPool();

                DescriptorPool( const DescriptorPool& ) = delete;
                DescriptorPool& operator=( const DescriptorPool& ) = delete;

                VkDescriptorSet allocateBufferDescriptorSets( DescriptorBufferAllocateInfo& );

                VkDescriptorPool pool;
            private:
                std::shared_ptr<VulkanDevice> device;
        };
    }
}
