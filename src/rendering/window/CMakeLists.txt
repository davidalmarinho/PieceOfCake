find_package(Vulkan REQUIRED)
find_package(glfw3 3.2 REQUIRED)

add_library(window
	Window.cpp
)

target_include_directories(window
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
)

target_link_libraries(window
	Vulkan::Vulkan
	glfw 
)
