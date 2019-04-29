#pragma once

#include "Device.h"

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

                void fillBuffer( void* data, size_t size );

                VkBuffer buffer;
                VkDeviceMemory memory;
                uint32_t size;
            private:
                std::shared_ptr<VulkanDevice> device;

                uint32_t getMemoryType( uint32_t Type_filter, VkMemoryPropertyFlags properties );
        };
    }
}
