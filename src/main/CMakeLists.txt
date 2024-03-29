add_library(engine
	Engine.cpp
)

target_include_directories(engine
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/components/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/rendering/textures/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
	"${PROJECT_SOURCE_DIR}/include/main/"
)

target_link_libraries(engine
	Vulkan::Vulkan
	glm::glm
	dear_imgui
)

add_executable(${PROJECT_NAME}
	main.cpp
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE 
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/components/"
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/rendering/textures/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/main/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
)

target_link_libraries(${PROJECT_NAME}
	# Global libraries
	engine
	rendering
	textures
	utils
	window
	input_device
	ecs
	components
	gui
)

# Libraries loading and linking
message("-=-=-=-=- Checking and loading libs -=-=-=-=-")

# GLFW -- Load with GLEW help
find_package(PkgConfig REQUIRED) 
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)    

# Check if Glfw was been fund
if (GLFW_FOUND)
	message(STATUS "${GREEN}> GLFW lib has been found.${COLOR_RESET}")
	include_directories(glfw)
	target_link_libraries(${PROJECT_NAME} glfw GLEW::GLEW)
else()
	message(STATUS "${RED}> Couldn't find GLFW lib!${COLOR_RESET}")
endif (GLFW_FOUND)


# Vulkan
find_package(Vulkan REQUIRED)
# Check if Vulkan was been found
if (VULKAN_FOUND)
	message(STATUS "${GREEN}> Vulkan lib has been found.${COLOR_RESET}")
	target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
else()
	message(STATUS "${RED}> Couldn't find Vulkan lib!${COLOR_RESET}")
endif(VULKAN_FOUND)

# Check if glm was been found
find_package(glm REQUIRED)
if (glm_FOUND)
	message(STATUS "${GREEN}> glm lib has been found.${COLOR_RESET}")
	target_link_libraries(${PROJECT_NAME} glm::glm)
else()
	message(STATUS "${RED}> Couldn't find glm lib!${COLOR_RESET}")
endif(glm_FOUND)

message("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-")

