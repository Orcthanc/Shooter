#include "Swapchain.h"
#include "Util.h"

#include <iostream>

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

    createImageViews();
}

VulkanSwapchain::~VulkanSwapchain(){
    for( auto& view : img_views )
        vkDestroyImageView( device->device, view, nullptr );

    vkDestroySwapchainKHR( device->device, swapchain, nullptr );
}

void VulkanSwapchain::checkPresentMode( VkPresentModeKHR& present_mode ){
    uint32_t present_mode_count;

    throwonerror( vkGetPhysicalDeviceSurfacePresentModesKHR( device->phys_dev, device->instance->surface, &present_mode_count, nullptr ), "Could not get avaible presentmodes", VK_SUCCESS );

    vector<VkPresentModeKHR> present_modes( present_mode_count );

    throwonerror( vkGetPhysicalDeviceSurfacePresentModesKHR( device->phys_dev, device->instance->surface, &present_mode_count, &present_modes[0] ), "Could not get avaible presentmodes", VK_SUCCESS );

    if( present_mode_count == 0 )
        throw runtime_error( "Could not find any presentmodes" );

    for( auto& mode: present_modes ){
        if( mode == present_mode )
            return;
    }

    cout << "Warning: Falling back to FIFO present-mode since the targeted one is not avaible\n";

    for( auto& mode: present_modes ){
        if( ( present_mode = mode ) == VK_PRESENT_MODE_FIFO_KHR )
            return;
    }

    throw runtime_error( "Could not get FIFO presentmode. This should never happen. Please check your graphics driver and vulkan installation." );
}


void VulkanSwapchain::checkNumImages( uint32_t& num_img, const VkSurfaceCapabilitiesKHR& capa ){
    num_img = capa.minImageCount > num_img ? capa.minImageCount : num_img;
    if( capa.maxImageCount > 0 )
        num_img = capa.maxImageCount < num_img ? capa.maxImageCount : num_img;
}

void VulkanSwapchain::checkSurfaceFormat( VkSurfaceFormatKHR& format ){
    uint32_t formats_count;

    throwonerror( vkGetPhysicalDeviceSurfaceFormatsKHR( device->phys_dev, device->instance->surface, &formats_count, nullptr ), "Could not get surface formats", VK_SUCCESS );

    vector<VkSurfaceFormatKHR> formats( formats_count );

    throwonerror( vkGetPhysicalDeviceSurfaceFormatsKHR( device->phys_dev, device->instance->surface, &formats_count, &formats[0] ), "Could not get surface formats", VK_SUCCESS );

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

void VulkanSwapchain::checkImageSize( VkExtent2D& format, const VkSurfaceCapabilitiesKHR& capa ){
    if( 0xFFFFFFFF == capa.currentExtent.width ){
        format.width = MIN( capa.maxImageExtent.width, MAX( capa.minImageExtent.width, format.width ));
        format.height = MIN( capa.maxImageExtent.height, MAX( capa.minImageExtent.height, format.height ));
    }else {
        format = capa.currentExtent;
    }
}

void VulkanSwapchain::createImageViews(){
    img_views.resize( imgs.size() );

    for( size_t i = 0; i < imgs.size(); ++i ){
        VkImageViewCreateInfo info = {
            //sType
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            //pNext
            nullptr,
            //flags
            0,
            //image
            imgs[i],
            //Image type (e.g. 1D, 2D, 3D, cubemap)
            VK_IMAGE_VIEW_TYPE_2D,
            //format
            surface_format.format,
            //components (swizzle colorchannels)
            {
                //r
                VK_COMPONENT_SWIZZLE_IDENTITY,
                //g
                VK_COMPONENT_SWIZZLE_IDENTITY,
                //b
                VK_COMPONENT_SWIZZLE_IDENTITY,
                //a
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            //subresource-range (mipmaps, ...)
            {
                //image aspect mask
                VK_IMAGE_ASPECT_COLOR_BIT,
                //mipmap level
                0,
                //level count
                1,
                //array-layers
                0,
                //layer-count
                1,
            },
        };

        throwonerror( vkCreateImageView( device->device, &info, nullptr, &img_views[i] ), "Could not create an image view", VK_SUCCESS );
    }
}

