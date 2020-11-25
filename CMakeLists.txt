# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("shadowply")

# Include sub-projects.
add_subdirectory ("shadowply")


#set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/libs/ffmpeg/lib)
#target_include_directories(shadowply PRIVATE ${FFMPEG_DIR})
# target_link_libraries(shadowply PRIVATE "${AVCODEC_LIBRARY}")
# link_directories(${CMAKE_SOURCE_DIR}/libs/ffmpeg)

#find_package(FFmpeg REQUIRED
#	COMPONENTS avcodec avfilter avdevice avutil swscale avformat swresample)
#include_directories(${CMAKE_SOURCE_DIR}/libs/ffmpeg)
