#pragma once

#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

namespace Shooter {
    namespace Renderer {
        class VulkanSemaphore {
            public:
                VulkanSemaphore( std::shared_ptr<VulkanDevice>& );
                ~VulkanSemaphore();

                std::shared_ptr<VulkanDevice> device;
                VkSemaphore semaphore;
        };
    }
}
