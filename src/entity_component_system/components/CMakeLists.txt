# find_package(glfw3 3.3 REQUIRED)

add_library(components
	Transform.cpp
)

target_include_directories(components
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/components/"
)

# target_link_libraries(input_device
# 	glfw
# )
