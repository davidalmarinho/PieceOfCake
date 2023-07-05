find_package(glfw3 3.3 REQUIRED)

add_library(input_device
	KeyListener.cpp
)

target_include_directories(input_device
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/input_device/"
)

target_link_libraries(input_device
	glfw
)
