#include "Renderer/Pipeline.h"

#include "Renderer/PipelineShaderStage.h"
#include "Util.h"

#include <iostream>

using namespace std;
using namespace Shooter::Renderer;

VulkanPipeline::VulkanPipeline( PipelineCreateInfo&& cr_inf ){
    device = cr_inf.device;
    swapchain = cr_inf.swapchain;

    //Shaders
    vector<VkShaderModule> shader_modules;

    for( auto& p: cr_inf.s_cr_inf.shader_module_paths ){
        shader_modules.push_back( createShaderModule( device->device, p ));
    }

    vector<VkPipelineShaderStageCreateInfo> shader_stage_cr_infos;

    createShaderStageInfos( shader_modules, cr_inf.s_cr_inf.shader_stages, cr_inf.s_cr_inf.shader_entry_points, shader_stage_cr_infos );

    //Functions
    //TODO
    VkPipelineVertexInputStateCreateInfo vertex_buffer_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &cr_inf.v_in_inf.binding_description,
        static_cast<uint32_t>( cr_inf.v_in_inf.attribute_descriptions.size() ),
        &cr_inf.v_in_inf.attribute_descriptions[0],
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE,
    };

    VkViewport viewport = {
        0.0f,
        0.0f,
        ( float )swapchain->img_size.width,
        ( float )swapchain->img_size.height,
        0.0f,
        1.0f,
    };

    VkRect2D scissor = {
        {
            0,
            0,
        },
        swapchain->img_size,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterization = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        VK_FALSE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blending_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &color_blend_attachment,
        {
            0.0f,
            0.0f,
            0.0f,
            0.0f,
        }
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>( cr_inf.l_cr_inf.descriptor_layouts.size() ),
        cr_inf.l_cr_inf.descriptor_layouts.size() > 0 ? &cr_inf.l_cr_inf.descriptor_layouts[0] : nullptr,
        0,
        nullptr,
    };

    throwonerror( vkCreatePipelineLayout( device->device, &pipeline_layout_info, nullptr, &pipeline_layout ), "Failed to create a Pipeline-layout", VK_SUCCESS );

    //Passes
    VkAttachmentDescription color_attachment = {
        0,
        swapchain->surface_format.format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_reference = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &color_attachment_reference,
        nullptr,
        nullptr,
        0,
        0,
    };

    VkRenderPassCreateInfo render_pass_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &color_attachment,
        1,
        &subpass,
        0,
        nullptr,
    };

    throwonerror( vkCreateRenderPass( device->device, &render_pass_info, nullptr, &render_pass ), "Could not create renderpass", VK_SUCCESS );

    //Creation
    VkGraphicsPipelineCreateInfo pipeline_cr_inf = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        &shader_stage_cr_infos[0],
        &vertex_buffer_info,
        &input_assembly_info,
        nullptr,
        &viewport_state,
        &rasterization,
        &multisampling,
        nullptr,
        &color_blending_info,
        nullptr,
        pipeline_layout,
        render_pass,
        0,
        VK_NULL_HANDLE,
        -1,
    };

    throwonerror( vkCreateGraphicsPipelines( device->device, VK_NULL_HANDLE, 1, &pipeline_cr_inf, nullptr, &pipeline ), "Failed to create Graphics pipeline", VK_SUCCESS );

    //Cleanup shadermodules
    for( auto& m: shader_modules )
        vkDestroyShaderModule( device->device, m, nullptr );

    //Create framebuffer
    framebuffers.resize( swapchain->img_views.size() );

    for( size_t i = 0; i < swapchain->img_views.size(); i++ ){
        VkImageView attachments[] = { swapchain->img_views[i] };

        VkFramebufferCreateInfo fb_cr_inf = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            render_pass,
            1,
            attachments,
            swapchain->img_size.width,
            swapchain->img_size.height,
            1
        };

        throwonerror( vkCreateFramebuffer( device->device, &fb_cr_inf, nullptr, &framebuffers[i] ), "Could not create framebuffer", VK_SUCCESS );
    }
}

VulkanPipeline::~VulkanPipeline(){
    for( auto fb: framebuffers )
        vkDestroyFramebuffer( device->device, fb, nullptr );

    vkDestroyPipeline( device->device, pipeline, nullptr );
    vkDestroyPipelineLayout( device->device, pipeline_layout, nullptr );
    vkDestroyRenderPass( device->device, render_pass, nullptr );
}
