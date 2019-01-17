#include "Util.h"
#include "Renderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <memory>

using namespace std;
using namespace Shooter::Renderer;

int main( int argc, char** argv ){

    GLFWwindow* window;
    try {
        throwonerror(glfwInit(), "can't init glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        throwonerror(window = glfwCreateWindow(width, height, NAME, NULL, NULL), "can't create window");

		InitSettings main_window_settings = {
			//GLFW-Window
			window,
			//Optional -- Existing VulkanInstance to use
			nullptr,
			//Instance Extensions
			vector<const char*> {},
			//Device Extensions
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
