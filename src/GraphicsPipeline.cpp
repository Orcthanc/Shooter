#include "GraphicsPipeline.h"
#include "Util.h"

#include <fstream>
#include <exception>

using namespace std;
using namespace Shooter::Renderer;

VulkanPipeline::VulkanPipeline( const PipelineInfo& pipeline_info ){
	//TODO
	throw runtime_error( "Unimplemented Method VulkanPipeline::VulkanPipeline" );
}

ShaderCode Shooter::Renderer::readFile( const string& path ){
	ifstream file( path, ios::ate | ios::binary );

	if( !file.is_open() )
		throw runtime_error( "Could not open file: " + path );

	ShaderCode code( file.tellg() );

	file.seekg( 0 );
	file.read( &code[0], code.size() );
	file.close();

	return code;
}

VkShaderModule Shooter::Renderer::createShaderModule( const VkDevice device, const ShaderCode& code ){
	VkShaderModuleCreateInfo cr_inf = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		//pNext
		nullptr,
		//flags
		0,
		code.size(),
		reinterpret_cast<const uint32_t*>(&code[0]),
	};

	VkShaderModule mod;

	throwonerror( vkCreateShaderModule( device, &cr_inf, nullptr, &mod ), "Could not create Shadermodule", VK_SUCCESS );

	return mod;
}

VkShaderModule Shooter::Renderer::createShaderModule( const VkDevice device, const string& path ){
	return createShaderModule( device, readFile( path ));
}

ShaderStageInfo::ShaderStageInfo( const VkDevice device, const std::string& path, VkShaderStageFlagBits stage ){
	this->stage = stage;
	code = createShaderModule( device, path );
}

ShaderStageInfo::ShaderStageInfo( const VkDevice device, const ShaderCode& sh_code, VkShaderStageFlagBits stage ){
	this->stage = stage;
	code = createShaderModule( device, sh_code );
}
