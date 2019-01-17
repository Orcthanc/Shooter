#include "Util.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>

int main( int argc, char** argv ){

    GLFWwindow* window;
    try {
        throwonerror(glfwInit(), "can't init glfw");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        throwonerror(window = glfwCreateWindow(1280, 720, "Shooter", NULL, NULL), "can't create window");

        while (!glfwWindowShouldClose(window)) {



            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    }catch(const std::exception& e){
        std::cout << "Error: " << std::endl << e.what() << std::endl;

    }
    glfwTerminate();
}
