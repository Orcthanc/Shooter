#pragma once

#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdint.h>

#include <vector>

namespace Shooter {
    namespace Renderer {
        class DescriptorSetLayout {
            public:
                DescriptorSetLayout( std::shared_ptr<VulkanDevice>&, std::vector<VkDescriptorSetLayoutBinding>& bindings );
                ~DescriptorSetLayout();

                VkDescriptorSetLayout layout;

            private:
                std::shared_ptr<VulkanDevice> device;
        };
    }
}
