#include "Renderer.h"

#include <string.h>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace Shooter::Renderer;

VulkanDevice::VulkanDevice( const InitSettings& settings ){
	if( !settings.existing_instance )
		createInstance( settings.desired_extensions );
	else
		instance = *settings.existing_instance;
}

VulkanDevice::~VulkanDevice(){

}


void VulkanDevice::createInstance( const std::vector<const char*> desiredExts ){
	active_extensions = getAvaibleExtensions( desiredExts, true );
}

static bool ext_supported( vector<VkExtensionProperties> avaible, const char* ext_name ){
	for( auto& av_ext: avaible )
		if( !strcmp( av_ext.extensionName, ext_name ))
			return true;
	return false;
}

vector<const char*> VulkanDevice::getAvaibleExtensions( vector<const char*> desired, bool addGLFWRequired ){
	vector<const char*> result;

	vector<VkExtensionProperties> avaible_exts;
	uint32_t avaible_ext_count;

	vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, nullptr );

	avaible_exts.resize( avaible_ext_count );
	vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, &avaible_exts[0] );

	if( addGLFWRequired ){
		uint32_t glfw_ext_count;

		const char** glfw_exts = glfwGetRequiredInstanceExtensions( &glfw_ext_count );

		for( uint32_t i = 0; i < glfw_ext_count; ++i ){
			if( ext_supported( avaible_exts, glfw_exts[i] ))
				result.push_back( glfw_exts[i] );
			else
				cout << "Warning: Could not find extension: " << glfw_exts[i] << endl;
		}
	}

	for( auto& ext: desired ){
		if( ext_supported( avaible_exts, ext ))
			result.push_back( ext );
		else
			cout << "Warning: Could not find extension: " << ext << endl;
	}

	return result;
}
