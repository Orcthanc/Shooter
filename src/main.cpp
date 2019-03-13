#include "Util.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "GraphicsPipeline.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <memory>

using namespace std;
using namespace Shooter::Renderer;

int main( int argc, char** argv ){

    GLFWwindow* window;

    try {
        throwonerror(glfwInit(), "can't init glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        throwonerror(window = glfwCreateWindow(width, height, NAME, NULL, NULL), "can't create window");

/*		InitSwapchainSettings swapchain_settings = {
			VK_PRESENT_MODE_MAILBOX_KHR,
			3,
			{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ width, height },
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			VK_NULL_HANDLE,
		};

		InitSettings main_window_settings = {
			//GLFW-Window
			window,
			//Create a presentation surface?
			true,
			//Optional -- Existing VulkanInstance to use
			nullptr,
			//Instance Extensions
			vector<const char*> {},
			//Device Extensions
			vector<const char*> {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			},
			//Require presentable queue-family
			true,
			//Queue-flags
			VK_QUEUE_GRAPHICS_BIT,
			&swapchain_settings,
		};

		unique_ptr<VulkanDevice> vulkan_device( new VulkanDevice( main_window_settings ));

		auto test = createShaderModule( vulkan_device->device, "res/shader/test1/vert.spv" );
		(void) test;
*/
		uint32_t glfw_ext_count;
		const char** c_glfw_ext = glfwGetRequiredInstanceExtensions( &glfw_ext_count );
		vector<const char*> glfw_exts( c_glfw_ext, c_glfw_ext + glfw_ext_count );
		InstanceInitSettings instance_settings = {
			glfw_exts,
			true,
			window,
		};
		shared_ptr<VulkanInstance> instance( new VulkanInstance( instance_settings ));

		DeviceInitSettings device_settings = {
			instance,
			{ VK_KHR_SWAPCHAIN_EXTENSION_NAME },
			true,
			0,
		};
		shared_ptr<VulkanDevice> device( new VulkanDevice( device_settings ));

		SwapchainInitSettings swapchain_settings = {
			device,
			//presentMode
			VK_PRESENT_MODE_MAILBOX_KHR,
			//images
			3,
			//format
			{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			//size
			{ width, height },
			//flags
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			//transforms
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			//old swapchain
			VK_NULL_HANDLE,
		};
		shared_ptr<VulkanSwapchain> swapchain( new VulkanSwapchain( swapchain_settings ));

		auto test = createShaderModule( device->device, "res/shader/test1/shader.vert.spv" );
		vkDestroyShaderModule( device->device, test, nullptr );



        while (!glfwWindowShouldClose(window)) {

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;

    }
    glfwTerminate();
}
