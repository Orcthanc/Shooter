#pragma once

#include "Device.h"

#include <memory>

namespace Shooter {
    namespace Renderer {
        struct TextureCreateInfo {
            public:
                std::shared_ptr<VulkanDevice>& device;
                std::string filename;
                VkFormat format;
                VkQueue& queue;
        };

        class Texture2D {
            public:
                Texture2D( TextureCreateInfo&& );
                Texture2D( const Texture2D& ) = delete;
                Texture2D& operator=( const Texture2D& ) = delete;
                ~Texture2D();

                std::shared_ptr<VulkanDevice> device;
                VkExtent2D extent;
                uint32_t miplevels;
                uint32_t layer_count;
                VkImageLayout layout;

                VkImage image;

            private:
                VkDeviceMemory memory;
        };
    }
}
