#include "Renderer/DescriptorSetLayout.h"

#include "Util.h"

#include <iostream>

using namespace std;
using namespace Shooter::Renderer;

DescriptorSetLayout::DescriptorSetLayout( shared_ptr<VulkanDevice>& device, vector<VkDescriptorSetLayoutBinding>& bindings ){
    this->device = device;

    VkDescriptorSetLayoutCreateInfo cr_inf = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>( bindings.size() ),
        &bindings[0],
    };

    throwonerror( vkCreateDescriptorSetLayout( this->device->device, &cr_inf, nullptr, &layout ), "Could not create descriptor-set", VK_SUCCESS );
}

DescriptorSetLayout::~DescriptorSetLayout(){
    vkDestroyDescriptorSetLayout( device->device, layout, nullptr );
}
