add_library(rendering
	FamilyQueues.cpp
	Application.cpp
	SwapChain.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
)

add_subdirectory(window)
