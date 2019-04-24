#include "Semaphore.h"
#include "Util.h"

using namespace std;
using namespace Shooter::Renderer;

VulkanSemaphore::VulkanSemaphore( shared_ptr<VulkanDevice>& device ){
    this->device = device;
    VkSemaphoreCreateInfo cr_inf = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0,
    };
    throwonerror( vkCreateSemaphore( this->device->device, &cr_inf, nullptr, &semaphore ), "Could not create semaphore", VK_SUCCESS );
}

VulkanSemaphore::~VulkanSemaphore(){
    vkDestroySemaphore( device->device, semaphore, nullptr );
}
