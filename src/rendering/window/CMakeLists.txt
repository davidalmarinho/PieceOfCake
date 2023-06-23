find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_library(window
	Window.cpp
)

target_include_directories(window
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
)

target_link_libraries(window
	Vulkan::Vulkan
	glfw 
)
