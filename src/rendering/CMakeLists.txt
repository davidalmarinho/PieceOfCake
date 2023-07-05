find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_library(rendering
	Renderer.cpp
	VulkanDebugger.cpp
	Pipeline.cpp
	SwapChain.cpp
	QueueFamilyIndices.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
	"${PROJECT_SOURCE_DIR}/include/main/"
)

target_link_libraries(rendering
	Vulkan::Vulkan
	glm::glm
)

add_subdirectory(window)
