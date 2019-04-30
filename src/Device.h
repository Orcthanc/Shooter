#pragma once

#include "Instance.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace Shooter{
    namespace Renderer{
        struct DeviceInitSettings{
            std::shared_ptr<VulkanInstance>& instance;
            const std::vector<const char*> extensions;
            bool require_presentable_queue;
            VkQueueFlags required_flags;
        };

        class VulkanDevice{
            public:
                VulkanDevice( const DeviceInitSettings& );
                ~VulkanDevice();

                VulkanDevice( const VulkanDevice& ) = delete;
                VulkanDevice& operator=( const VulkanDevice& );

                uint32_t getPhysicalDeviceQueueFamilyIndex( VkQueueFlagBits queue_type );

                VkDevice device;
                VkPhysicalDevice phys_dev;
                std::shared_ptr<VulkanInstance> instance;
                uint32_t present_queue_index;
            private:
                void getRequiredQueueFamilies( const DeviceInitSettings&, VkPhysicalDevice& phys_dev, std::vector<VkDeviceQueueCreateInfo>& create_infos );
                void selectPhysicalDevice( const DeviceInitSettings&, VkPhysicalDevice& phys_dev );

                std::vector<VkQueueFamilyProperties> properties;
        };
    }
}
