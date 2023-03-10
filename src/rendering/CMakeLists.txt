add_library(rendering
	Application.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
)

add_subdirectory(window)
