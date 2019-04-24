#include "Util.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Semaphore.h"

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
            VK_QUEUE_GRAPHICS_BIT,
        };
        shared_ptr<VulkanDevice> device( new VulkanDevice( device_settings ));

        SwapchainInitSettings swapchain_settings = {
            device,
            //presentMode
            VK_PRESENT_MODE_MAILBOX_KHR,
            //images
            3,
            //format
            { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
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

        PipelineCreateInfo pipeline_settings = {
            device,
            swapchain,
            {
                {
                    "res/shader/test1/shader.vert.spv",
                    "res/shader/test1/shader.frag.spv",
                },
                {
                    VK_SHADER_STAGE_VERTEX_BIT,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                },
                {
                    "main",
                    "main",
                },
            },
        };

        shared_ptr<VulkanPipeline> pipeline( new VulkanPipeline( pipeline_settings ));

        VulkanCommandPool command_pool( device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), 0);

        command_pool.allocCommandBuffers( swapchain->imgs.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY );


//Should probably be moved to somewhere else, but don't know to where
        VkClearValue background = {
            {
                {
                    0.392f,
                    0.584f,
                    0.929f,
                    1.0f,
                },
            },
        };

        for( size_t i = 0; i < command_pool.buffers.size(); i++ ){
            VkCommandBufferBeginInfo begin_inf = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                nullptr,
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                nullptr,
            };

            throwonerror( vkBeginCommandBuffer( command_pool.buffers[i], &begin_inf ), "Could not start recording of a command-buffer", VK_SUCCESS );

            VkRenderPassBeginInfo render_inf = {
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                nullptr,
                pipeline->render_pass,
                pipeline->framebuffers[i],
                {
                    { 0, 0 },
                    swapchain->img_size,
                },
                1,
                &background,
            };

            vkCmdBeginRenderPass( command_pool.buffers[i], &render_inf, VK_SUBPASS_CONTENTS_INLINE );
            vkCmdBindPipeline( command_pool.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline );
            vkCmdDraw( command_pool.buffers[i], 3, 1, 0, 0 );
            vkCmdEndRenderPass( command_pool.buffers[i] );
            throwonerror( vkEndCommandBuffer( command_pool.buffers[i] ), "Could not record command-buffer", VK_SUCCESS );
        }
//end of should be moved
        VulkanSemaphore render_start( device );
        VulkanSemaphore render_end( device );

        VkQueue graphics_queue;
        VkQueue present_queue;
        vkGetDeviceQueue( device->device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), 0, &graphics_queue );
        vkGetDeviceQueue( device->device, device->present_queue_index, 0, &present_queue );

        while (!glfwWindowShouldClose(window)) {

//          glfwSwapBuffers(window);
            glfwPollEvents();

            //Testing purposes
            uint32_t image_index;
            vkAcquireNextImageKHR( device->device, swapchain->swapchain, 0xFFFFFFFFFFFFFFFF, render_start.semaphore, VK_NULL_HANDLE, &image_index );

            VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

            VkSubmitInfo submit_inf = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,
                nullptr,
                //Start rendering when semaphore signaled
                1,
                &render_start.semaphore,
                wait_stages,
                //Stuff to render
                1,
                &command_pool.buffers[image_index],
                //Signal when done
                1,
                &render_end.semaphore,
            };


            throwonerror( vkQueueSubmit( graphics_queue, 1, &submit_inf, VK_NULL_HANDLE ), "Couldn't submit queue", VK_SUCCESS );

            VkPresentInfoKHR present_inf = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                nullptr,
                1,
                &render_end.semaphore,
                1,
                &swapchain->swapchain,
                &image_index,
                nullptr,
            };

            vkQueuePresentKHR( present_queue, &present_inf );

            vkQueueWaitIdle( present_queue );
        }

        vkDeviceWaitIdle( device->device );

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;

    }
    glfwTerminate();
}
