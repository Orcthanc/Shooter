#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <memory>
#include <vector>

namespace Shooter{
    namespace Renderer{
        typedef std::vector<char> ShaderCode;
        
        extern ShaderCode readFile( const std::string& path );
        extern VkShaderModule createShaderModule( const VkDevice, const ShaderCode& );
        extern VkShaderModule createShaderModule( const VkDevice, const std::string& path );

        extern void createShaderStageInfos( const std::vector<VkShaderModule>& modules, const std::vector<VkShaderStageFlagBits>& stages, const std::vector<std::string>& entryPoints, std::vector<VkPipelineShaderStageCreateInfo>& result );
    }
}
