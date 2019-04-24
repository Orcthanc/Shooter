#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Shooter{
    namespace Renderer{
        struct InstanceInitSettings{
            public:
                const std::vector<const char*> extensions;
                bool create_surface;
                GLFWwindow* window;
        };

        class VulkanInstance{
            public:
                VulkanInstance( const InstanceInitSettings& );
                ~VulkanInstance();

                VkInstance instance;
                VkSurfaceKHR surface;

            private:
                bool checkExtensions( const std::vector<const char*>& extensions );
        };
    }
}
