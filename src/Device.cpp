#include "Device.h"
#include "Util.h"

#include <string.h>

#include <algorithm>
#include <map>

using namespace std;
using namespace Shooter::Renderer;

static bool ext_supported( vector<VkExtensionProperties> avaible, const char* ext_name ){
	for( auto& av_ext: avaible )
		if( !strcmp( av_ext.extensionName, ext_name ))
			return true;
	return false;
}

VulkanDevice::VulkanDevice( const DeviceInitSettings& settings ){
	instance = settings.instance;

	VkPhysicalDevice phys_dev;
	selectPhysicalDevice( settings, phys_dev );

	vector<VkDeviceQueueCreateInfo> queue_create_infos;
	getRequiredQueueFamilies( settings, phys_dev, queue_create_infos );

	VkDeviceCreateInfo dev_cr_inf = {
		//sType
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		//pNext
		nullptr,
		//flags
		0,
		//desired queue_families
		static_cast<uint32_t>( queue_create_infos.size() ),
		queue_create_infos.size() > 0 ? &queue_create_infos[0] : nullptr,
		//VulkanLayers
		0,
		nullptr,
		//desired extensions
		static_cast<uint32_t>( settings.extensions.size() ),
		settings.extensions.size() > 0 ? &settings.extensions[0] : nullptr,
		//Desired features
		{},
	};

	this->phys_dev = phys_dev;

	throwonerror( vkCreateDevice( phys_dev, &dev_cr_inf, nullptr, &device ), "Could not create logical vulkan device", VK_SUCCESS );

}

void VulkanDevice::getRequiredQueueFamilies( const DeviceInitSettings& settings, VkPhysicalDevice& phys_dev, vector<VkDeviceQueueCreateInfo>& create_infos ){
	uint32_t queue_family_count = 0;
	vector<VkQueueFamilyProperties> properties;

	vkGetPhysicalDeviceQueueFamilyProperties( phys_dev, &queue_family_count, nullptr );
	properties.resize( queue_family_count );

	vkGetPhysicalDeviceQueueFamilyProperties( phys_dev, &queue_family_count, &properties[0] );
	if( properties.size() == 0 || queue_family_count == 0 )
		throw std::runtime_error( "Could not get Queue Family Properties for any Family" );

	vector<uint32_t> indices;

	if( settings.require_presentable_queue ){

		uint32_t queue_family_index = 0;

		for( ; queue_family_index < properties.size(); ++queue_family_index ){
			VkBool32 surface_presentation_support = VK_FALSE;
			VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR( phys_dev, queue_family_index, instance->surface, &surface_presentation_support );

			if( res == VK_SUCCESS && surface_presentation_support == VK_TRUE )
				break;
		}

		indices.push_back( queue_family_index );
	}

	VkQueueFlags all_queue_flags[] = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_SPARSE_BINDING_BIT };

	for( uint8_t i = 0; i < 4; ++i ){
		for( uint32_t j = 0; j < static_cast<uint32_t>( properties.size() ); ++j ){
			if( properties[j].queueCount > 0 && ( properties[j].queueFlags & settings.required_flags & all_queue_flags[i] )){
				indices.push_back( j );
				break;
			}
		}
	}

	sort( indices.begin(), indices.end() );
	auto last = unique( indices.begin(), indices.end() );
	indices.erase( last, indices.end() );

	//TODO? use better priorities than max
	for( size_t i = 0; i < indices.size(); ++i ){
		float priorities[] = { 1.0f };
		create_infos.push_back( {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			indices[i],
			static_cast<uint32_t>( 1 ),
			priorities,
		} );
	}
}

void VulkanDevice::selectPhysicalDevice( const DeviceInitSettings& settings, VkPhysicalDevice& phys_dev ){
	vector<VkPhysicalDevice> devices;
	uint32_t device_amount;

	throwonerror( vkEnumeratePhysicalDevices( instance->instance, &device_amount, nullptr ), "Failed to enumerate physical devices", VK_SUCCESS );
	devices.resize( device_amount );
	throwonerror( vkEnumeratePhysicalDevices( instance->instance, &device_amount, &devices[0] ), "Failed to enumerate physical devices", VK_SUCCESS );

	uint32_t desired_device_index = 0;
	bool extensions_supported;

	multimap<int, VkPhysicalDevice> device_scores;

	for( ;desired_device_index < device_amount; ++desired_device_index ){
		int score = 0;

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

		for( auto& ext: settings.extensions ){
			if( !ext_supported( avaible_device_extensions, ext ))
				extensions_supported = false;
		}
		if( !extensions_supported )
			continue;

		if( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
			score += 1000;

		score += properties.limits.maxImageDimension2D;

		device_scores.insert( make_pair( score, devices[desired_device_index] ));
	}

	if( device_scores.rbegin()->first == 0 )
		throw runtime_error( "Could not find a suitable GPU" );

	phys_dev = device_scores.rbegin()->second;
}

