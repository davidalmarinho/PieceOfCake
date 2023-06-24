find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_library(rendering
	VulkanDebugger.cpp
	Pipeline.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
)

target_link_libraries(rendering
	Vulkan::Vulkan
	glm::glm
)

add_subdirectory(window)
