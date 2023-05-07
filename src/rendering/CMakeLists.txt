add_library(rendering
	FamilyQueues.cpp
	Application.cpp
	SwapChain.cpp
)

target_include_directories(rendering
	PRIVATE
	"${PROJECT_SOURCE_DIR}/include/rendering/"
	"${PROJECT_SOURCE_DIR}/include/utils/"
)

add_subdirectory(window)
