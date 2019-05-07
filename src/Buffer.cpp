#include "Renderer/Buffer.h"

#include "Renderer/CommandPool.h"
#include "Util.h"

#include <string.h>

using namespace std;
using namespace Shooter::Renderer;

Buffer::Buffer( BufferCreateInfo&& cr_inf ){
    device = cr_inf.device;

    size = cr_inf.size;

    VkBufferCreateInfo buffer_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        cr_inf.size,
        cr_inf.usage,
        VK_SHARING_MODE_EXCLUSIVE,
    };

    throwonerror( vkCreateBuffer( device->device, &buffer_info, nullptr, &buffer ), "Could not create Buffer", VK_SUCCESS );


    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements( device->device, buffer, &mem_reqs );

    VkMemoryAllocateInfo alloc_inf = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        mem_reqs.size,
        getMemoryType( mem_reqs.memoryTypeBits, cr_inf.required_properties ),
    };

    throwonerror( vkAllocateMemory( device->device, &alloc_inf, nullptr, &memory ), "Could not allocate buffer memory", VK_SUCCESS );

    throwonerror( vkBindBufferMemory( device->device, buffer, memory, 0 ), "Could not bind buffer memory", VK_SUCCESS );
}

Buffer::~Buffer(){
    vkDestroyBuffer( device->device, buffer, nullptr );
    vkFreeMemory( device->device, memory, nullptr );
}

uint32_t Buffer::getMemoryType( uint32_t type_filter, VkMemoryPropertyFlags properties ){
    VkPhysicalDeviceMemoryProperties phys_dev_mem_props;

    vkGetPhysicalDeviceMemoryProperties( device->phys_dev, &phys_dev_mem_props );

    for( uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++ ){
        if(( type_filter & ( 1 << i )) && (( phys_dev_mem_props.memoryTypes[i].propertyFlags & properties ) == properties )){
            return i;
        }
    }
    throw runtime_error( "Could not find a suitible buffer memory type." );
}

void Buffer::fillBuffer( void* data, size_t size ){
    void* buffer_data;
    vkMapMemory( device->device, memory, 0, size, 0, &buffer_data );
    memcpy( buffer_data, data, size );
    vkUnmapMemory( device->device, memory );
}

void Buffer::copyDataTo( Buffer& target, VulkanCommandPool& cmd_pool, size_t copy_size, VkQueue queue ){
    cmd_pool.allocCommandBuffers( 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY );
    
    VkCommandBuffer& cmd_buffer = cmd_pool.buffers[0];

    VkCommandBufferBeginInfo begin_inf = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr,
    };

    vkBeginCommandBuffer( cmd_buffer, &begin_inf );

    VkBufferCopy region = {
        0,
        0,
        copy_size,
    };

    vkCmdCopyBuffer( cmd_buffer, buffer, target.buffer, 1, &region );

    vkEndCommandBuffer( cmd_buffer );

    VkSubmitInfo sub_inf = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &cmd_buffer,
        0,
        nullptr,
    };

    vkQueueSubmit( queue, 1, &sub_inf, VK_NULL_HANDLE );
    vkQueueWaitIdle( queue );
}

void Buffer::fillDeviceLocalBuffer( void* data, size_t size, VulkanCommandPool& cmd_pool, VkQueue transfer_queue ){
    BufferCreateInfo stage_buffer_cr_inf = {
        device,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        size,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    };

    Buffer staging_buffer( move( stage_buffer_cr_inf ));
    staging_buffer.fillBuffer( data, size );

    staging_buffer.copyDataTo( *this, cmd_pool, size, transfer_queue );
}
