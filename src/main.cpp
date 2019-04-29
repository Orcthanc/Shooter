#include "Util.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Semaphore.h"
#include "Vertex.h"
#include "Buffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <exception>
#include <memory>

using namespace std;
using namespace Shooter::Renderer;

void createRenderPasses( VulkanCommandPool& command_pool, VulkanPipeline& pipeline, VulkanSwapchain& swapchain, Buffer& vertex_buffer ){
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
            pipeline.render_pass,
            pipeline.framebuffers[i],
            {
                { 0, 0 },
                swapchain.img_size,
            },
            1,
            &background,
        };

        vkCmdBeginRenderPass( command_pool.buffers[i], &render_inf, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline( command_pool.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline );
        VkBuffer vertexbuffers[] = {
            vertex_buffer.buffer,
        };
        VkDeviceSize offsets[] = {
            0,
        };
        vkCmdBindVertexBuffers( command_pool.buffers[i], 0, 1, vertexbuffers, offsets );

        vkCmdDraw( command_pool.buffers[i], 3, 1, 0, 0 );
        vkCmdEndRenderPass( command_pool.buffers[i] );
        throwonerror( vkEndCommandBuffer( command_pool.buffers[i] ), "Could not record command-buffer", VK_SUCCESS );
    }

}

void draw( VulkanDevice& device, VulkanSwapchain& swapchain, VulkanCommandPool& command_pool, VkQueue& graphics_queue, VkQueue& present_queue, VulkanSemaphore& render_start, VulkanSemaphore& render_end ){
    uint32_t image_index;
    vkAcquireNextImageKHR( device.device, swapchain.swapchain, 0xFFFFFFFFFFFFFFFF, render_start.semaphore, VK_NULL_HANDLE, &image_index );

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
        &swapchain.swapchain,
        &image_index,
        nullptr,
    };

    vkQueuePresentKHR( present_queue, &present_inf );

    vkQueueWaitIdle( present_queue );
}

int main( int argc, char** argv ){

    GLFWwindow* window;

    try {
        throwonerror(glfwInit(), "can't init glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        throwonerror(window = glfwCreateWindow(width, height, NAME, NULL, NULL), "can't create window");

        uint32_t glfw_ext_count;
        const char** c_glfw_ext = glfwGetRequiredInstanceExtensions( &glfw_ext_count );
        vector<const char*> glfw_exts( c_glfw_ext, c_glfw_ext + glfw_ext_count );

        InstanceInitSettings instance_settings = {
            //required extensions
            glfw_exts,
            //create a presentation surface
            true,

            window,
        };
        shared_ptr<VulkanInstance> instance( new VulkanInstance( instance_settings ));

        DeviceInitSettings device_settings = {
            instance,
            //device extensions
            { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
            //Require presentable queue?
            true,
            //Required Queues
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
            {
                SimpleVertex::getBindingDescription( 0 ),
                SimpleVertex::getAttributeDescriptions( 0 ),
            },
        };

        shared_ptr<VulkanPipeline> pipeline( new VulkanPipeline( pipeline_settings ));

        VulkanCommandPool command_pool( device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), 0 );

        command_pool.allocCommandBuffers( swapchain->imgs.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY );

        VkQueue graphics_queue;
        VkQueue present_queue;
        vkGetDeviceQueue( device->device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), 0, &graphics_queue );
        vkGetDeviceQueue( device->device, device->present_queue_index, 0, &present_queue );




        const vector<SimpleVertex> vertices = {
            {{  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
            {{  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }},
            {{ -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }},
        };

        BufferCreateInfo b_cr_inf = {
            device,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sizeof( vertices[0] ) * vertices.size(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        };

        Buffer buffer( b_cr_inf );
        { //Load data to buffer

            VulkanCommandPool staging_pool( device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT );

            //Testdata

            buffer.fillDeviceLocalBuffer( (void*)&vertices[0], sizeof( vertices[0] ) * vertices.size(), staging_pool, graphics_queue );
            /*
            BufferCreateInfo b_cr_inf_stage = {
                device,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                sizeof( vertices[0] ) * vertices.size(),
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            };

            Buffer staging_buffer( b_cr_inf_stage );
            staging_buffer.fillBuffer( (void*)&vertices[0], sizeof( vertices[0] ) * vertices.size() );

            staging_buffer.copyDataTo( buffer, staging_pool, sizeof( vertices[0] ) * vertices.size(), graphics_queue );
            */
        }//End Load data
        //End Testdata


        createRenderPasses( command_pool, *pipeline, *swapchain, buffer );


        VulkanSemaphore render_start( device );
        VulkanSemaphore render_end( device );


        while (!glfwWindowShouldClose(window)) {

//          glfwSwapBuffers(window);
            glfwPollEvents();

            draw( *device, *swapchain, command_pool, graphics_queue, present_queue, render_start, render_end );
        }

        vkDeviceWaitIdle( device->device );

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;
    }
    glfwTerminate();
}
