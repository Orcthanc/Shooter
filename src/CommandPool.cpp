#include "CommandPool.h"
#include "Util.h"

using namespace std;
using namespace Shooter::Renderer;

VulkanCommandPool::VulkanCommandPool( shared_ptr<VulkanDevice>& device, uint32_t family_index, VkCommandPoolCreateFlags flags ){
    this->device = device;
    VkCommandPoolCreateInfo cr_inf = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        flags,
        family_index,
    };

    throwonerror( vkCreateCommandPool( this->device->device, &cr_inf, nullptr, &command_pool ), "Could not create CommandPool.", VK_SUCCESS );
}

VulkanCommandPool::~VulkanCommandPool(){
    vkDestroyCommandPool( device->device, command_pool, nullptr );
}

void VulkanCommandPool::allocCommandBuffers( uint32_t amount, VkCommandBufferLevel level ){
    buffers.resize( amount );

    VkCommandBufferAllocateInfo alloc_inf = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        command_pool,
        level,
        amount,
    };

    throwonerror( vkAllocateCommandBuffers( device->device, &alloc_inf, &buffers[0] ), "Could not create command-buffers", VK_SUCCESS );
}
