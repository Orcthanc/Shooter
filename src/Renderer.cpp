#include "Renderer.h"
#include "Util.h"

#include <string.h>

#include <vector>
#include <algorithm>
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

	if( settings.swapchain_settings )
		createSwapchain( *settings.swapchain_settings );
}

VulkanDevice::~VulkanDevice(){
	if( *swapchain != VK_NULL_HANDLE ){
		vkDestroySwapchainKHR( *device, *swapchain, nullptr );
		*swapchain = VK_NULL_HANDLE;
	}

	vkDestroyDevice( *device, nullptr );

	if( instance.use_count() == 1 )
		vkDestroyInstance( *instance, nullptr );
}

void VulkanDevice::checkPresentMode( VkPresentModeKHR& present_mode ){
	uint32_t present_mode_count;

	throwonerror( vkGetPhysicalDeviceSurfacePresentModesKHR( phys_dev, *surface, &present_mode_count, nullptr ), "Could not get avaible presentmodes", VK_SUCCESS );

	vector<VkPresentModeKHR> present_modes( present_mode_count );

	throwonerror( vkGetPhysicalDeviceSurfacePresentModesKHR( phys_dev, *surface, &present_mode_count, &present_modes[0] ), "Could not get avaible presentmodes", VK_SUCCESS );

	if( present_mode_count == 0 )
		throw new runtime_error( "Could not find any presentmodes" );

	for( auto& mode: present_modes ){
		if( mode == present_mode )
			return;
	}

	cout << "Warning: Falling back to FIFO present-mode since the targeted one is not avaible\n";

	for( auto& mode: present_modes ){
		if( ( present_mode = mode ) == VK_PRESENT_MODE_FIFO_KHR )
			return;
	}

	throw new runtime_error( "Could not get FIFO presentmode. This should never happen. Please check your graphics driver and vulkan installation." );
}


void VulkanDevice::checkNumImages( uint32_t& num_img, const VkSurfaceCapabilitiesKHR& capa ){
	num_img = capa.minImageCount > num_img ? capa.minImageCount : num_img;
	if( capa.maxImageCount > 0 )
		num_img = capa.maxImageCount < num_img ? capa.maxImageCount : num_img;
}

void VulkanDevice::checkSurfaceFormat( VkSurfaceFormatKHR& format ){
	uint32_t formats_count;

	throwonerror( vkGetPhysicalDeviceSurfaceFormatsKHR( phys_dev, *surface, &formats_count, nullptr ), "Could not get surface formats", VK_SUCCESS );

	vector<VkSurfaceFormatKHR> formats( formats_count );

	throwonerror( vkGetPhysicalDeviceSurfaceFormatsKHR( phys_dev, *surface, &formats_count, &formats[0] ), "Could not get surface formats", VK_SUCCESS );

	if( formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED )
		return;

	for( auto& f: formats ){
		if( f.format == format.format && f.colorSpace == format.colorSpace )
			return;
	}

	cout << "Warning: Falling back to a random colorspace.\n";

	for( auto& f: formats ){
		if( f.format == format.format ){
			format.colorSpace = f.colorSpace;
			return;
		}
	}

	cout << "Warning: Falling back to a random format.\n";

	format.format = formats[0].format;
	format.colorSpace = formats[0].colorSpace;
}

void VulkanDevice::checkImageSize( VkExtent2D& format, const VkSurfaceCapabilitiesKHR& capa ){
	if( 0xFFFFFFFF == capa.currentExtent.width ){
		format.width = MIN( capa.maxImageExtent.width, MAX( capa.minImageExtent.width, format.width ));
		format.height = MIN( capa.maxImageExtent.height, MAX( capa.minImageExtent.height, format.height ));
	}else {
		format = capa.currentExtent;
	}
}

void VulkanDevice::createSwapchain( const InitSwapchainSettings& desired_settings ){
	InitSwapchainSettings settings( desired_settings );

	VkSurfaceCapabilitiesKHR surface_capabilities;

	throwonerror( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( phys_dev, *surface, &surface_capabilities ), "Could not get surface-capabilities", VK_SUCCESS );

	checkPresentMode( settings.desired_present_mode );
	checkNumImages( settings.desired_num_images, surface_capabilities );
	checkSurfaceFormat( settings.desired_format );
	checkImageSize( settings.desired_img_size, surface_capabilities );

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		//sType
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		//pNext
		nullptr,
		//flags
		0,
		//surface
		*surface,
		//minImageCount
		settings.desired_num_images,
		//imageFormat
		settings.desired_format.format,
		//imageColorSpace
		settings.desired_format.colorSpace,
		//imageExtent
		settings.desired_img_size,
		//imageArrayLayers (for layered/stereoscopic rendering)
		1,
		//image usage flags
		settings.flags,
		//imageSharingMode
		VK_SHARING_MODE_EXCLUSIVE,
		//queueFamilyIndexCount
		0,
		//pQueueFamilyIndices
		nullptr,
		//preTransform
		settings.transform_flags,
		//compositeAlpha
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		//presentMode
		settings.desired_present_mode,
		//clipped
		VK_TRUE,
		//oldSwapchain
		settings.old_swapchain,
	};

	swapchain = make_unique<VkSwapchainKHR>();

	throwonerror( vkCreateSwapchainKHR( *device, &swapchain_create_info, nullptr, swapchain.get() ), "Could not create swapchain", VK_SUCCESS );

	if( swapchain == VK_NULL_HANDLE )
		throw new runtime_error( "Could not create swapchain" );

	if( settings.old_swapchain != VK_NULL_HANDLE ){
		vkDestroySwapchainKHR( *device, settings.old_swapchain, nullptr );
		settings.old_swapchain = VK_NULL_HANDLE;
	}
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

void VulkanDevice::getRequiredQueueFamilies( const InitSettings& settings, VkPhysicalDevice& phys_dev, vector<VkDeviceQueueCreateInfo>& create_infos ){
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
			VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR( phys_dev, queue_family_index, *surface, &surface_presentation_support );

			if( res == VK_SUCCESS && surface_presentation_support == VK_TRUE )
				break;
		}

		indices.push_back( queue_family_index );
	}

	VkQueueFlags all_queue_flags[] = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_SPARSE_BINDING_BIT };

	for( uint8_t i = 0; i < 4; ++i ){
		for( uint32_t j = 0; j < static_cast<uint32_t>( properties.size() ); ++j ){
			if( properties[j].queueCount > 0 && ( properties[j].queueFlags & settings.required_queue_flags & all_queue_flags[i] )){
				indices.push_back( j );
				break;
			}
		}
	}

	sort( indices.begin(), indices.end() );
	auto last = unique( indices.begin(), indices.end() );
	indices.erase( last, indices.end() );

	//TODO use better priorities than max
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

void VulkanDevice::createDevice( const InitSettings& settings ){
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
		static_cast<uint32_t>( settings.desired_device_extensions.size() ),
		settings.desired_device_extensions.size() > 0 ? &settings.desired_device_extensions[0] : nullptr,
		//Desired features
		{},
	};

	this->phys_dev = phys_dev;

	device = make_unique<VkDevice>();
	throwonerror( vkCreateDevice( phys_dev, &dev_cr_inf, nullptr, device.get() ), "Could not create logical vulkan device", VK_SUCCESS );
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
