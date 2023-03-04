# Project configuration
cmake_minimum_required(VERSION 3.10)                                                                          
project(PieceCake)                                                                                     
      
set(CMAKE_CXX_STANDARD 17)                                                                                    
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_MODULE_PATH}")                                           
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_PREFIX_PATH}")                                           
                                                                                                              
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")                                               
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")                                               
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")                                               

# Set color variables
if(NOT WIN32)
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
endif()

set(SOURCE_FILES main.cpp)

add_subdirectory(src)

# add_executable(${PROJECT_NAME} ${SOURCE_FILES})
                                                                                                             
# TODO: Don't forget to copy the assets folder to the out directory
#add_custom_target(copy_assets                                                                                
#    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_LIBRARY_OUTPUT_DIRECTO      RY}/assets"                                                                                                   
#)                                                                                                            
                                                                                                              
