add_executable(${PROJECT_NAME}
	main.cpp
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE 
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
)

target_link_libraries(${PROJECT_NAME}
	# Global libraries
	utils

	rendering
	window
)

# Libraries loading and linking
message("-=-=-=-=- Checking and loading libs -=-=-=-=-")

# GLFW -- Load with GLEW help
find_package(PkgConfig REQUIRED) 
find_package(glfw3 3.2 REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)    

# Check if Glfw was been fund
if (GLFW_FOUND)
	message(STATUS "${GREEN}> GLFW lib has been found.${COLOR_RESET}")
	include_directories(${GLFW_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES} GLEW)
	target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
else()
	message(STATUS "${RED}> Couldn't find GLFW lib!${COLOR_RESET}")
endif (GLFW_FOUND)


# Vulkan
find_package(Vulkan REQUIRED)
# Check if Vulkan was been found
if (VULKAN_FOUND)
	message(STATUS "${GREEN}> Vulkan lib has been found.${COLOR_RESET}")
	target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
else()
	message(STATUS "${RED}> Couldn't find Vulkan lib!${COLOR_RESET}")
endif(VULKAN_FOUND)
message("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-")

