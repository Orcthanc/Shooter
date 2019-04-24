#pragma once

#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

namespace Shooter {
    namespace Renderer {
        class VulkanCommandPool {
            public:
                VulkanCommandPool( std::shared_ptr<VulkanDevice>&, uint32_t queue_family_index, uint32_t flags );
                ~VulkanCommandPool();

                void allocCommandBuffers( uint32_t amount, VkCommandBufferLevel );

                
                VkCommandPool command_pool;
                std::shared_ptr<VulkanDevice> device;
                std::vector<VkCommandBuffer> buffers;
            private:
        };
    }
}
