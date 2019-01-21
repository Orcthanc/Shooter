#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace Shooter{
	namespace Renderer{
		struct InitSettings{
			//Window the surface should use
			GLFWwindow* glfw_window;
			//Create a presentation surface on the window
			bool create_surface;
			//Either nullptr or a specific VkInstance
			std::shared_ptr<VkInstance> existing_instance;
			//Extensions to be loaded
			std::vector<const char*> desired_instance_extensions;
			std::vector<const char*> desired_device_extensions;
		};

		class VulkanDevice{
			public:
				VulkanDevice( const InitSettings& );
				~VulkanDevice();

				//All loaded extensions
				std::vector<const char*> active_extensions;
				GLFWwindow* glfw_window;
				std::shared_ptr<VkInstance> instance;
				std::shared_ptr<VkSurfaceKHR> surface;
			private:
				//Returns all avaible extensions out of desired and (if set to true) also the extensions required by glfw
				std::vector<const char*> getAvaibleExtensions( std::vector<const char*> desired, bool addGLFWRequired );
				//Creates a VkInstance and safes it in instance
				void createInstance( const std::vector<const char*> desiredExts );
				//Creates a logical device
				void createDevice( const InitSettings& settings );
				//Finds a Physical device suitable for rendering
				void selectPhysicalDevice( const InitSettings& settings, VkPhysicalDevice& phys_dev );
				//Returns all needed QueueFamilies
				void getRequiredQueueFamilies( const InitSettings& settings, VkPhysicalDevice& phys_dev );
		};
	}
}
