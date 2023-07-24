find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_library(components
	Transform.cpp
	PerspectiveCamera.cpp
	ModelRenderer.cpp
)

target_include_directories(components
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/components/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
)

target_link_libraries(components
	glm::glm
	Vulkan::Vulkan
)
