#include "PipelineShaderStage.h"
#include "Util.h"

#include <fstream>
#include <exception>

using namespace std;
using namespace Shooter::Renderer;

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

void Shooter::Renderer::createShaderStageInfos( const vector<VkShaderModule>& modules, const vector<VkShaderStageFlagBits>& stages, const vector<string>& entry_points, vector<VkPipelineShaderStageCreateInfo>& result ){
    for( size_t i = 0; i < modules.size(); ++i ){
        result.push_back({
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            stages[i],
            modules[i],
            entry_points[i].c_str(),
            nullptr,
        });
    }
}
