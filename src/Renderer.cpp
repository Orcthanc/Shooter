#include "Renderer.h"
#include "Util.h"

#include <string.h>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace Shooter::Renderer;

static bool ext_supported( vector<VkExtensionProperties> avaible, const char* ext_name ){
	for( auto& av_ext: avaible )
		if( !strcmp( av_ext.extensionName, ext_name ))
			return true;
	return false;
}

VulkanDevice::VulkanDevice( const InitSettings& settings ){
	glfw_window = settings.glfw_window;

	if( !settings.existing_instance ){
		createInstance( settings.desired_instance_extensions );
	}else {
		instance = settings.existing_instance;
	}

	if( settings.create_surface ){
		surface = make_shared<VkSurfaceKHR>();
		throwonerror( glfwCreateWindowSurface( *instance, glfw_window, NULL, surface.get() ), "Could not create a presentation-surface", VK_SUCCESS );
	}

	createDevice( settings );
}

VulkanDevice::~VulkanDevice(){
	if( instance.use_count() == 1 )
		vkDestroyInstance( *instance, nullptr );
}

void VulkanDevice::selectPhysicalDevice( const InitSettings& settings, VkPhysicalDevice& phys_dev ){
	vector<VkPhysicalDevice> devices;
	uint32_t device_amount;

	throwonerror( vkEnumeratePhysicalDevices( *instance, &device_amount, nullptr ), "Failed to enumerate physical devices", VK_SUCCESS );
	devices.resize( device_amount );
	throwonerror( vkEnumeratePhysicalDevices( *instance, &device_amount, &devices[0] ), "Failed to enumerate physical devices", VK_SUCCESS );

	uint32_t desired_device_index = 0;
	bool extensions_supported;

	for( ;desired_device_index < device_amount; ++desired_device_index ){
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceProperties properties;

		vkGetPhysicalDeviceFeatures( devices[desired_device_index], &features );
		vkGetPhysicalDeviceProperties( devices[desired_device_index], &properties );

		//TODO check better if device is sufficient
		extensions_supported = true;

		vector<VkExtensionProperties> avaible_device_extensions;
		uint32_t avaible_ext_count;

		throwonerror( vkEnumerateDeviceExtensionProperties( devices[desired_device_index], nullptr, &avaible_ext_count, nullptr ), "Could not get Extensions of a device", VK_SUCCESS );

		avaible_device_extensions.resize( avaible_ext_count );
		throwonerror( vkEnumerateDeviceExtensionProperties( devices[desired_device_index], nullptr, &avaible_ext_count, &avaible_device_extensions[0] ), "Could not get Extensions of a device", VK_SUCCESS );

		for( auto& ext: settings.desired_device_extensions ){
			if( !ext_supported( avaible_device_extensions, ext ))
				extensions_supported = false;
		}
		if( extensions_supported )
			break;
	}

	throwonerror( extensions_supported, "Could not find a physical vulkan device (Graphics Card) with the required extensions" );

	phys_dev = devices[desired_device_index];
}


void VulkanDevice::getRequiredQueueFamilies( const InitSettings& settings, VkPhysicalDevice& phys_dev ){
	uint32_t queue_family_count = 0;
	vector<VkQueueFamilyProperties> properties;

	vkGetPhysicalDeviceQueueFamilyProperties( phys_dev, &queue_family_count, nullptr );
	properties.resize( queue_family_count );

	vkGetPhysicalDeviceQueueFamilyProperties( phys_dev, &queue_family_count, &properties[0] );
	if( properties.size() == 0 || queue_family_count == 0 )
		throw std::runtime_error( "Could not get Queue Family Properties for any Family" );

	uint32_t queue_family_index = 0;

	for( ; queue_family_index < properties.size(); ++queue_family_index ){
		VkBool32 surface_presentation_support = VK_FALSE;
		VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR( phys_dev, queue_family_index, *surface, &surface_presentation_support );

		if( res == VK_SUCCESS && surface_presentation_support == VK_TRUE )
			break;
	}

	//TODO
}

void VulkanDevice::createDevice( const InitSettings& settings ){
	VkPhysicalDevice phys_dev;
	selectPhysicalDevice( settings, phys_dev );

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	getRequiredQueueFamilies( settings, phys_dev );

	//TODO
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
