#pragma once

#include <glm/glm.hpp>

namespace Shooter {
    namespace Renderer {
        struct MVPMat {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };
    }
}
