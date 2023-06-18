add_library(utils
	AssetPool.cpp
)

target_include_directories(utils
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/utils/"
)
