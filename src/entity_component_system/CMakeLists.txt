# find_package(glfw3 3.3 REQUIRED)

add_library(ecs
	ECS.cpp
)

target_include_directories(ecs
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
)

add_subdirectory(components)

# target_link_libraries(input_device
# 	glfw
# )
