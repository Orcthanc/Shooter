#include "Renderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <memory>

using namespace std;
using namespace Shooter::Renderer;

template <typename T>
T throwonerror(T x, const char* error){
    if(!x)
        throw std::runtime_error(error);
    return x;
}

int main( int argc, char** argv ){

    GLFWwindow* window;
    try {
        throwonerror(glfwInit(), "can't init glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        throwonerror(window = glfwCreateWindow(1280, 720, "Shooter", NULL, NULL), "can't create window");

		InitSettings main_window_settings = {
			window,
			vector<const char*> {},
		};

		unique_ptr<VulkanDevice> vulkan_device( new VulkanDevice( main_window_settings ));

        while (!glfwWindowShouldClose(window)) {



            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;

    }
    glfwTerminate();
}
