#pragma once

#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Shooter{
    namespace Renderer{
        struct SwapchainInitSettings{
            std::shared_ptr<VulkanDevice>& device;

            VkPresentModeKHR desired_present_mode;
            uint32_t desired_num_images;
            VkSurfaceFormatKHR desired_format;
            VkExtent2D desired_img_size;
            VkImageUsageFlags flags;
            VkSurfaceTransformFlagBitsKHR transform_flags;
            //Probably VK_NULL_HANDLE
            VkSwapchainKHR old_swapchain;
        };

        class VulkanSwapchain{
            public:
                VulkanSwapchain( const SwapchainInitSettings& );
                ~VulkanSwapchain();

                VulkanSwapchain( const VulkanSwapchain& ) = delete;
                VulkanSwapchain& operator=( const VulkanSwapchain& ) = delete;

                std::shared_ptr<VulkanDevice> device;
                VkSwapchainKHR swapchain;
                std::vector<VkImage> imgs;
                std::vector<VkImageView> img_views;
                
                VkPresentModeKHR present_mode;
                uint32_t num_of_imgs;
                VkSurfaceFormatKHR surface_format;
                VkExtent2D img_size;
                VkImageUsageFlags flags;
                VkSurfaceTransformFlagBitsKHR transform_flags;
            private:
                void checkPresentMode( VkPresentModeKHR& );
                void checkNumImages( uint32_t&, const VkSurfaceCapabilitiesKHR& );
                void checkSurfaceFormat( VkSurfaceFormatKHR& );
                void checkImageSize( VkExtent2D&, const VkSurfaceCapabilitiesKHR& );
                void createImageViews();
        };
    }
}
