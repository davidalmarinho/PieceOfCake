find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_library(rendering
	DescriptorLayout.cpp
	Shader.cpp
	Model.cpp
	Renderer.cpp
	VulkanDebugger.cpp
	Pipeline.cpp
	SwapChain.cpp
	QueueFamilyIndices.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/rendering/textures/"
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
	"${PROJECT_SOURCE_DIR}/include/main/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/tiny_obj_loader/"
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
)

target_link_libraries(rendering
	Vulkan::Vulkan
	glm::glm
)

add_subdirectory(window)
add_subdirectory(textures)