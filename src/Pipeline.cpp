#include "Pipeline.h"

#include "PipelineShaderStage.h"
#include "Util.h"

using namespace std;
using namespace Shooter::Renderer;

VulkanPipeline::VulkanPipeline( PipelineCreateInfo& cr_inf ){
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
		0,
		nullptr,
		0,
		nullptr,
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
		VK_FRONT_FACE_CLOCKWISE,
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
		0,
		nullptr,
		0,
		nullptr,
	};

	throwonerror( vkCreatePipelineLayout( device->device, &pipeline_layout_info, nullptr, &pipeline_layout ), "Failed to create a Pipeline-layout", VK_SUCCESS );

	//Passes
	
	//Creation
	
	//Cleanup
	for( auto& m: shader_modules )
		vkDestroyShaderModule( device->device, m, nullptr );
}

VulkanPipeline::~VulkanPipeline(){
	vkDestroyPipelineLayout( device->device, pipeline_layout, nullptr );
}
