#pragma once

namespace Shooter{
	namespace Renderer{
		struct InitSettings{

		};
		
		class VulkanDevice{
			VulkanDevice( const InitSettings& );
			~VulkanDevice();
		};
	}
}
