find_package(Vulkan REQUIRED)

add_library(utils
	AssetPool.cpp
	Utils.cpp
)

target_include_directories(utils
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/utils/"
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/rendering/textures/"
	"${PROJECT_SOURCE_DIR}/libs/tiny_obj_loader/"
)

target_link_libraries(utils
	Vulkan::Vulkan
)