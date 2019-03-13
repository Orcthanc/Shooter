project( Shooter )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

file( GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp" )

add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} )

find_package( glfw3 3.2 REQUIRED )
find_package( Vulkan REQUIRED )


compile_shaders(
		shader/test1/shader.frag
		shader/test1/shader.vert
)

target_link_libraries( Shooter PUBLIC Vulkan::Vulkan glfw )
target_include_directories( Shooter PUBLIC "." )

if( UNIX )
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -O3" )
	target_compile_definitions( ${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XLIB_KHR=1 )
elseif( WIN32 )
	target_compile_definitions( ${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WIN32_KHR=1 )
endif( UNIX )
