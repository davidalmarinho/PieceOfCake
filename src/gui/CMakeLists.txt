add_library(gui
	ImGuiLayer.cpp
)

target_include_directories(gui
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/gui/"
)

target_link_libraries(gui
	dear_imgui
)
