find_package(Vulkan REQUIRED)

add_library(rendering
	VulkanDebugger.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
)

target_link_libraries(rendering
	Vulkan::Vulkan
)

add_subdirectory(window)
