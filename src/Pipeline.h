#pragma once

#include "Device.h"
#include "Swapchain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <memory>

namespace Shooter{
    namespace Renderer{
        struct PipelineShaderCreateInfo{
            public:
                std::vector<std::string> shader_module_paths;
                std::vector<VkShaderStageFlagBits> shader_stages;
                std::vector<std::string> shader_entry_points;
        };
        
        struct PipelineCreateInfo{
            public:
                std::shared_ptr<VulkanDevice>& device;
                std::shared_ptr<VulkanSwapchain>& swapchain;
                PipelineShaderCreateInfo s_cr_inf;
        };


        class VulkanPipeline{
            public:
                VulkanPipeline( PipelineCreateInfo& );
                ~VulkanPipeline();

                VkPipeline pipeline;
                std::vector<VkFramebuffer> framebuffers;
                VkRenderPass render_pass;

            private:
                std::shared_ptr<VulkanDevice> device;
                std::shared_ptr<VulkanSwapchain> swapchain;

                VkPipelineLayout pipeline_layout;
        };
    }
}
