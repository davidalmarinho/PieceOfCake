add_library(utils
	Compare.cpp
)

target_include_directories(utils
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/utils/"
)
