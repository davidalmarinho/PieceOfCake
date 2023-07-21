find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
# find_package(SDL2 REQUIRED)
# find_package(SDL2_image REQUIRED)

add_library(textures
	Texture.cpp
)

target_include_directories(textures
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/entity_component_system/"
	"${PROJECT_SOURCE_DIR}/include/rendering/textures/"
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/input_device/"
	"${PROJECT_SOURCE_DIR}/include/stb_image/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/main/"
)

target_link_libraries(textures
	Vulkan::Vulkan
	glfw 
)

# target_link_libraries(textures
# 	SDL2_image::SDL2_image
# 	SDL2::SDL2
# )
