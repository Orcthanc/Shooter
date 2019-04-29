#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

namespace Shooter {
    namespace Renderer {

        //Doesn't support virtual functions cause it must be POD :(
        struct SimpleVertex {
            public:
                glm::vec2 pos;
                glm::vec3 color;

                static VkVertexInputBindingDescription getBindingDescription( uint32_t binding );
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions( uint32_t binding );
        };
    }
}
