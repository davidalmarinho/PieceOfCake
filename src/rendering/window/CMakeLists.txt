add_library(window
	Window.cpp
)

target_include_directories(window
	PUBLIC
	"${PROJECT_SOURCE_DIR}/include/rendering/window/"
)
