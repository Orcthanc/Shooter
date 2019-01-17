#include "Renderer.h"
#include "Util.h"

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
		instance = settings.existing_instance;
}

VulkanDevice::~VulkanDevice(){
	if( instance.use_count() == 1 )
		vkDestroyInstance( *instance, nullptr );
}


void VulkanDevice::createInstance( const std::vector<const char*> desiredExts ){
	active_extensions = getAvaibleExtensions( desiredExts, true );

	VkApplicationInfo application_info = {
		//Struct Type
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		//pNext
		nullptr,
		//Name of Window
		NAME,
		//Vk Version
		VK_MAKE_VERSION( 1, 0, 0 ),
		//Name of Engine
		NAME " Engine",
		//Engine Version
		VK_MAKE_VERSION( 1, 0, 0 ),
		//Api Version
		VK_MAKE_VERSION( 1, 0, 0 ),
	};

	VkInstanceCreateInfo instance_create_info = {
		//Structure Type
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		//pNext
		nullptr,
		//flags
		0,
		//Selfexplanatory
		&application_info,
		//Enabled Layers
		0,
		nullptr,
		//Extensions to enable
		static_cast<uint32_t>( active_extensions.size() ),
		active_extensions.size() > 0 ? &active_extensions[0] : nullptr,
	};

	instance = make_shared<VkInstance>();
	throwonerror( vkCreateInstance( &instance_create_info, nullptr, instance.get() ), "Could not create VkInstance", VK_SUCCESS );
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

	throwonerror( vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, nullptr ), "Could not get instance extensions", VK_SUCCESS );

	avaible_exts.resize( avaible_ext_count );
	throwonerror( vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, &avaible_exts[0] ), "Could not get instance extensions", VK_SUCCESS );

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
