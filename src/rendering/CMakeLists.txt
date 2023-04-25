add_library(rendering
	FamilyQueues.cpp
	Application.cpp
	SwapChain.cpp
)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.2 REQUIRED)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	glfw
	Vulkan::Vulkan
)

target_link_libraries(rendering
	Vulkan::Vulkan
	glfw
)

add_subdirectory(window)
