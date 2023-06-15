# Project configuration
cmake_minimum_required(VERSION 3.20)                                                                          
project(PieceOfCake)                                                                                     
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_MODULE_PATH}")
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_PREFIX_PATH}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Set color variables
  string(ASCII 27 Esc)
  set(COLOR_RESET  "${Esc}[m")
  set(COLOR_BOLD   "${Esc}[1m")
  set(RED          "${Esc}[31m")
  set(GREEN        "${Esc}[32m")
  set(YELOOW       "${Esc}[33m")
  set(BLUE         "${Esc}[34m")
  set(MAGENTA      "${Esc}[35m")
  set(CYAN         "${Esc}[36m")
  set(WHITE        "${Esc}[37m")
  set(BOLD_RED     "${Esc}[1;31m")
  set(BOLD_GREEN   "${Esc}[1;32m")
  set(BOLD_YELLOW  "${Esc}[1;33m")
  set(BOLD_BLUE    "${Esc}[1;34m")
  set(BOLD_MAGENTA "${Esc}[1;35m")
  set(BOLD_CYAN    "${Esc}[1;36m")
  set(BOLD_WHITE   "${Esc}[1;37m")

# Windows setting
if(WIN32)
  add_definitions(-DGLEW_STATIC)
  add_definitions(/std:c++17)
endif()
  
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  
set(SOURCE_FILES main.cpp)

# WIndows setting
if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(src)

# Windows setting
if(WIN32)
  # set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY
  #  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # TODO: Not working. Test in x86 architecture is needed.
  #if(X86)
  #  set(VCPKG_TARGET_TRIPLET "x86-windows-static")
  #else()
  #  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  #endif()
endif()

# Linux and Mac setting
if(NOT WIN32)
   target_compile_options("${PROJECT_NAME}" 
    PRIVATE 
    -g
  )
endif()

# Delete previous binary
file(REMOVE_RECURSE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

message("${CYAN}> Running in ${CMAKE_BUILD_TYPE} mode.${COLOR_RESET}")

# Copying compiled shaders to bin directory
if(WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(INSTALL out/shaders DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
  else()
    file(INSTALL out/shaders DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release")
  endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
  file(INSTALL out/shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif(WIN32)

# add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# TODO: Don't forget to copy the assets folder to the out directory
#add_custom_target(copy_assets                                                                                
#    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_LIBRARY_OUTPUT_DIRECTO      RY}/assets"                                                                                                   
#)                                                                                                            
                                                                                                              
