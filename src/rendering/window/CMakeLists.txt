add_library(window
	Window.cpp
)

#find_package(glfw3 3.2 REQUIRED)
#find_package(Vulkan REQUIRED)

target_include_directories(window
	PUBLIC
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	#glfw
	#Vulkan::Vulkan
)

#target_link_libraries(window
#	glfw
#	Vulkan::Vulkan
#)
