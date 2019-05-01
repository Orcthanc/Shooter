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

        struct PipelineVertexInputInfo {
            public:
                VkVertexInputBindingDescription binding_description;
                std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        };

        struct PipelineLayoutCreateInfo {
            public:
                std::vector<VkDescriptorSetLayout> descriptor_layouts;
        };
        
        struct PipelineCreateInfo{
            public:
                std::shared_ptr<VulkanDevice>& device;
                std::shared_ptr<VulkanSwapchain>& swapchain;
                PipelineShaderCreateInfo s_cr_inf;
                PipelineVertexInputInfo v_in_inf;
                PipelineLayoutCreateInfo l_cr_inf;
        };


        class VulkanPipeline{
            public:
                VulkanPipeline( PipelineCreateInfo& );
                ~VulkanPipeline();

                VulkanPipeline( const VulkanPipeline& ) = delete;
                VulkanPipeline& operator=( const VulkanPipeline& ) = delete;

                VkPipeline pipeline;
                std::vector<VkFramebuffer> framebuffers;
                VkRenderPass render_pass;

                VkPipelineLayout pipeline_layout;

            private:
                std::shared_ptr<VulkanDevice> device;
                std::shared_ptr<VulkanSwapchain> swapchain;
        };
    }
}
