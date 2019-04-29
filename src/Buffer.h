#pragma once

#include "Device.h"
#include "CommandPool.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdint.h>

namespace Shooter {
    namespace Renderer {
        
        struct BufferCreateInfo {
            public:
                std::shared_ptr<VulkanDevice>& device;
                VkBufferUsageFlags usage;
                size_t size;
                VkMemoryPropertyFlags required_properties;
        };

        class Buffer {
            public:
                Buffer( const BufferCreateInfo& );
                ~Buffer();

                void fillDeviceLocalBuffer( void* data, size_t size, VulkanCommandPool& cmd_pool, VkQueue transfer_queue );
                void fillBuffer( void* data, size_t size );
                void copyDataTo( Buffer& target, VulkanCommandPool& cmd_pool, size_t copy_size, VkQueue transfer_queue );

                VkBuffer buffer;
                VkDeviceMemory memory;
                uint32_t size;
            private:
                std::shared_ptr<VulkanDevice> device;

                uint32_t getMemoryType( uint32_t Type_filter, VkMemoryPropertyFlags properties );
        };
    }
}
