#include "Util.h"
#include "Renderer/Instance.h"
#include "Renderer/Device.h"
#include "Renderer/Swapchain.h"
#include "Renderer/Pipeline.h"
#include "Renderer/CommandPool.h"
#include "Renderer/Semaphore.h"
#include "Renderer/Vertex.h"
#include "Renderer/Buffer.h"
#include "Renderer/Uniforms.h"
#include "Renderer/DescriptorSetLayout.h"
#include "Renderer/DescriptorPool.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <exception>
#include <memory>
#include <chrono>

using namespace std;
using namespace Shooter::Renderer;

void createRenderPasses( VulkanCommandPool& command_pool, VulkanPipeline& pipeline, VulkanSwapchain& swapchain, Buffer& vertex_buffer, Buffer& index_buffer, uint32_t index_buffer_size, VkDescriptorSet& descriptor_set ){
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
        VkBuffer vertex_buffers[] = {
            vertex_buffer.buffer,
        };
        VkDeviceSize offsets[] = {
            0,
        };
        vkCmdBindVertexBuffers( command_pool.buffers[i], 0, 1, vertex_buffers, offsets );
        vkCmdBindIndexBuffer( command_pool.buffers[i], index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16 );
        vkCmdBindDescriptorSets( command_pool.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_layout, 0, 1, &descriptor_set, 0, nullptr );

        vkCmdDrawIndexed( command_pool.buffers[i], index_buffer_size, 1, 0, 0, 0 );
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

static auto start_time = chrono::high_resolution_clock::now();
static auto last_time = chrono::high_resolution_clock::now();
void updateUniformBuffer( Buffer& uniform, MVPMat mvp ){
    auto curr_time = chrono::high_resolution_clock::now();

    float time = chrono::duration<float, chrono::seconds::period>( curr_time - start_time ).count();
    float fps = chrono::duration<float, chrono::seconds::period>( curr_time - last_time ).count();
    last_time = curr_time;
    printf( "\rCurrent fps: %f", 1 / fps );
    mvp.model = glm::rotate( mvp.model, time, glm::vec3( 0.0f, 0.0f, 1.0f ));

    uniform.fillBuffer( (void*)&mvp, sizeof( mvp ));


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

        vector<VkDescriptorSetLayoutBinding> bindings = {
            {
                {
                    0,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    1,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    nullptr,
                },
            },
        };
        
        DescriptorSetLayout descriptor_layout( device, bindings );

        BufferCreateInfo mvp_ub_cr_inf = {
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            sizeof( MVPMat ),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        Buffer mvp_uniform_buffer( mvp_ub_cr_inf );


        DescriptorPoolSizeInfo p_s_inf = {
            device,
            {
                {
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    1,
                },
            },
            0,
            1,
        };

        DescriptorPool desc_pool( p_s_inf );

        DescriptorBufferAllocateInfo d_b_alloc_inf = {
            {
                descriptor_layout.layout,
            },
            {
                {
                    mvp_uniform_buffer.buffer,
                    0,
                    sizeof( MVPMat ),
                },
            },
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        };

        VkDescriptorSet desc_set = desc_pool.allocateBufferDescriptorSets( d_b_alloc_inf );

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
            {
                {
                    descriptor_layout.layout,
                },
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
            {{ -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
            {{  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }},
            {{  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }},
            {{ -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }},
        };

        const vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0,
        };

        BufferCreateInfo b_cr_inf = {
            device,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sizeof( vertices[0] ) * vertices.size(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        };

        BufferCreateInfo ind_b_cr_inf = {
            device,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sizeof( indices[0] ) * indices.size(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        };

        
        Buffer vertex_buffer( b_cr_inf );
        Buffer index_buffer( ind_b_cr_inf );
        { //Load data to vertex_buffer

            VulkanCommandPool staging_pool( device, device->getPhysicalDeviceQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT ), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT );

            //Testdata

            vertex_buffer.fillDeviceLocalBuffer( (void*)&vertices[0], sizeof( vertices[0] ) * vertices.size(), staging_pool, graphics_queue );
            index_buffer.fillDeviceLocalBuffer( (void*)&indices[0], sizeof( indices[0] ) * indices.size(), staging_pool, graphics_queue );
        }//End Load data (Out of Scope staging_pool)
        //End Testdata




        createRenderPasses( command_pool, *pipeline, *swapchain, vertex_buffer, index_buffer, static_cast<uint32_t>( indices.size() ), desc_set );


        VulkanSemaphore render_start( device );
        VulkanSemaphore render_end( device );

        MVPMat test_mvp;

        test_mvp.model = glm::mat4( 1.0f );
        test_mvp.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ));
        test_mvp.proj = glm::perspective( glm::radians( 45.0f ), (float) swapchain->img_size.width / (float) swapchain->img_size.height, 0.1f, 50.0f );

        //Cause different coordinates than OpenGL
        test_mvp.proj[1][1] *= -1;


        while (!glfwWindowShouldClose(window)) {

//          glfwSwapBuffers(window);
            glfwPollEvents();

            updateUniformBuffer( mvp_uniform_buffer, test_mvp );
            draw( *device, *swapchain, command_pool, graphics_queue, present_queue, render_start, render_end );
        }

        vkDeviceWaitIdle( device->device );

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;
    }
    glfwTerminate();
}
