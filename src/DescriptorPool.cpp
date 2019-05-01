#include "DescriptorPool.h"

#include "Util.h"

#include <iostream>

using namespace std;
using namespace Shooter::Renderer;

DescriptorPool::DescriptorPool( DescriptorPoolSizeInfo& alloc_inf ){
    device = alloc_inf.device;

    VkDescriptorPoolCreateInfo cr_inf = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        alloc_inf.flags,
        alloc_inf.max_sets,
        static_cast<uint32_t>( alloc_inf.sets.size() ),
        &alloc_inf.sets[0],
    };

    throwonerror( vkCreateDescriptorPool( device->device, &cr_inf, nullptr, &pool ), "Could not create descriptor-pool", VK_SUCCESS );
}

DescriptorPool::~DescriptorPool(){
    vkDestroyDescriptorPool( device->device, pool, nullptr );
}

VkDescriptorSet DescriptorPool::allocateBufferDescriptorSets( DescriptorBufferAllocateInfo& alloc_inf ){
    VkDescriptorSet result;

    uint32_t size = static_cast<uint32_t>( alloc_inf.layouts.size() );

    VkDescriptorSetAllocateInfo all_inf = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        pool,
        size,
        &alloc_inf.layouts[0],
        //&alloc_inf.layouts[0].layout,
    };
    //TODO does this write a vector or one element containing an array?
    throwonerror( vkAllocateDescriptorSets( device->device, &all_inf, &result ), "Failed to allocate descriptor sets", VK_SUCCESS );

    VkWriteDescriptorSet write_cmd = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        result,
        0,
        0,
        size,
        alloc_inf.descriptor_type,
        nullptr,
        &alloc_inf.buffer_info[0],
        nullptr,
    };

    vkUpdateDescriptorSets( device->device, 1, &write_cmd, 0, nullptr );

    return result;
}
