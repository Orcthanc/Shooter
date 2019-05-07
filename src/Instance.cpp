#include "Renderer/Instance.h"
#include "Util.h"

#include <iostream>
#include <string.h>

using namespace std;
using namespace Shooter::Renderer;

static bool ext_supported( vector<VkExtensionProperties> avaible, const char* ext_name ){
    for( auto& av_ext: avaible )
        if( !strcmp( av_ext.extensionName, ext_name ))
            return true;
    return false;
}

VulkanInstance::VulkanInstance( InstanceInitSettings&& settings ){

    throwonerror( checkExtensions( settings.extensions ), "Could not find required instance extensions" );

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
        static_cast<uint32_t>( settings.extensions.size() ),
        settings.extensions.size() > 0 ? &settings.extensions[0] : nullptr,
    };

    throwonerror( vkCreateInstance( &instance_create_info, nullptr, &instance ), "Could not create VkInstance", VK_SUCCESS );

    if( settings.create_surface )
        throwonerror( glfwCreateWindowSurface( instance, settings.window, nullptr, &surface ), "Could not create a surface.", VK_SUCCESS );
    else
        surface = VK_NULL_HANDLE;

}

VulkanInstance::~VulkanInstance(){
    vkDestroyInstance( instance, nullptr );
}

bool VulkanInstance::checkExtensions( const vector<const char*>& exts ){
    vector<VkExtensionProperties> avaible_exts;
    uint32_t avaible_ext_count;

    throwonerror( vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, nullptr ), "Could not get instance extensions", VK_SUCCESS );

    avaible_exts.resize( avaible_ext_count );
    throwonerror( vkEnumerateInstanceExtensionProperties( nullptr, &avaible_ext_count, &avaible_exts[0] ), "Could not get instance extensions", VK_SUCCESS );

    for( auto& ext: exts ){
        if( !ext_supported( avaible_exts, ext )){
            cout << "Extension " << ext << " not supported!" << endl;
            return false;
        }
    }

    return true;

}
