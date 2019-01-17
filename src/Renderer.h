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
				std::shared_ptr<VkInstance> instance;
			private:
				//Returns all avaible extensions out of desired and (if set to true) also the extensions required by glfw
				std::vector<const char*> getAvaibleExtensions( std::vector<const char*> desired, bool addGLFWRequired );
				void createInstance( const std::vector<const char*> desiredExts );
				void createDevice( const InitSettings& settings );
				void selectPhysicalDevice( const InitSettings& settings, VkPhysicalDevice& phys_dev );
		};
	}
}
