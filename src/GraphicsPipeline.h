#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <memory>
#include <vector>

namespace Shooter{
	namespace Renderer{
		typedef std::vector<char> ShaderCode;
		
		struct ShaderStageInfo{
			public:
				VkShaderModule code;
				VkShaderStageFlagBits stage;
				std::string entryName = "main";

				ShaderStageInfo( const VkDevice device, const std::string& path, VkShaderStageFlagBits stage );
				ShaderStageInfo( const VkDevice device, const ShaderCode& code, VkShaderStageFlagBits stage );
				~ShaderStageInfo();
		};

		struct PipelineInfo{
			VkDevice device;
			std::vector<ShaderStageInfo> shader_stages;
		};

		class VulkanPipeline{
			public:
				VulkanPipeline( const PipelineInfo& );
		};

		extern ShaderCode readFile( const std::string& path );
		extern VkShaderModule createShaderModule( const VkDevice, const ShaderCode& );
		extern VkShaderModule createShaderModule( const VkDevice, const std::string& path );
	}
}
