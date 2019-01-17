#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Shooter{
	namespace Renderer{
		struct InitSettings{
			//Window the surface should use
			GLFWwindow* glfw_window;
			//Extensions to be loaded
			std::vector<const char*> desired_extensions;
			//Either nullptr or a specific VkInstance
			VkInstance* existing_instance;
		};
		
		class VulkanDevice{
			public:
				VulkanDevice( const InitSettings& );
				~VulkanDevice();
				
				//All loaded extensions
				std::vector<const char*> active_extensions;
				VkInstance instance;
			private:
				//Returns all avaible extensions out of desired and (if set to true) also the extensions required by glfw
				std::vector<const char*> getAvaibleExtensions( std::vector<const char*> desired, bool addGLFWRequired );
				void createInstance( const std::vector<const char*> desiredExts );
		};
	}
}
