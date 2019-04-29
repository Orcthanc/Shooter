#include "Vertex.h"

using namespace std;
using namespace Shooter::Renderer;

VkVertexInputBindingDescription SimpleVertex::getBindingDescription( uint32_t binding ){
    return {
        binding,
        sizeof( SimpleVertex ),
        VK_VERTEX_INPUT_RATE_VERTEX,
    };
}


vector<VkVertexInputAttributeDescription> SimpleVertex::getAttributeDescriptions( uint32_t binding ){
    return {
        {
            0,
            0,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof( SimpleVertex, pos ),
        },
        {
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof( SimpleVertex, color ),
        },
    };
}
