#include "Swapchain.h"
#include "Util.h"

using namespace std;
using namespace Shooter::Renderer;

VulkanSwapchain::VulkanSwapchain( const SwapchainInitSettings& settings ){
	device = settings.device;

	present_mode = settings.desired_present_mode;
	num_of_imgs = settings.desired_num_images;
	surface_format = settings.desired_format;
	img_size = settings.desired_img_size;
	flags = settings.flags;
	transform_flags = settings.transform_flags;

	VkSurfaceCapabilitiesKHR surface_capabilities;

	throwonerror( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device->phys_dev, device->instance->surface, &surface_capabilities ), "Could not get surface-capabilities", VK_SUCCESS );

	checkPresentMode( present_mode );
	checkNumImages( num_of_imgs, surface_capabilities );
	checkSurfaceFormat( surface_format );
	checkImageSize( img_size, surface_capabilities );

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		//sType
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		//pNext
		nullptr,
		//flags
		0,
		//surface
		device->instance->surface,
		//minImageCount
		num_of_imgs,
		//imageFormat
		surface_format.format,
		//imageColorSpace
		surface_format.colorSpace,
		//imageExtent
		img_size,
		//imageArrayLayers (for layered/stereoscopic rendering)
		1,
		//image usage flags
		flags,
		//imageSharingMode (EXCLUSIVE = nonparallel) (Concurrent requires at least 2 queue-families)
		VK_SHARING_MODE_EXCLUSIVE,
		//queueFamilyIndexCount
		0,
		//pQueueFamilyIndices
		nullptr,
		//preTransform
		transform_flags,
		//compositeAlpha (currently ignore alpha)
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		//presentMode
		present_mode,
		//clipped (True = don't draw things obscures by other windows in the windowing system)
		VK_TRUE,
		//oldSwapchain
		settings.old_swapchain,
	};

	throwonerror( vkCreateSwapchainKHR( device->device, &swapchain_create_info, nullptr, &swapchain ), "Could not create swapchain", VK_SUCCESS );

	if( swapchain == VK_NULL_HANDLE )
		throw runtime_error( "Could not create swapchain" );

	if( settings.old_swapchain != VK_NULL_HANDLE ){
		vkDestroySwapchainKHR( device->device, settings.old_swapchain, nullptr );
	}

	uint32_t swapchain_img_count;

	vkGetSwapchainImagesKHR( device->device, swapchain, &swapchain_img_count, nullptr );
	imgs.resize( swapchain_img_count );

	vkGetSwapchainImagesKHR( device->device, swapchain, &swapchain_img_count, &imgs[0] );

	//TODO create Img views
}
