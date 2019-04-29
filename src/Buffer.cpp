#include "Buffer.h"

#include "Util.h"

#include <string.h>

using namespace std;
using namespace Shooter::Renderer;

Buffer::Buffer( const BufferCreateInfo& cr_inf ){
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

    vkBindBufferMemory( device->device, buffer, memory, 0 );
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
