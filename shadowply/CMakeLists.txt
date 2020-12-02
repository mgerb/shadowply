# CMakeList.txt : CMake project for shadowply, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (shadowply "shadowply.c" "shadowply.h" "util.h" "util.c" "capture.c" "capture.h" "window_util.h" "window_util.c" "libav.h" "libav.c")

# TODO: Add tests and install targets if needed.


find_library(AVCODEC_LIBRARY 
	NAMES avcodec
	HINTS "${CMAKE_SOURCE_DIR}/libs/ffmpeg/lib"
)

find_library(AVUTIL_LIBRARY 
	NAMES avutil
	HINTS "${CMAKE_SOURCE_DIR}/libs/ffmpeg/lib"
)

find_library(AVFORMAT_LIBRARY 
	NAMES avformat
	HINTS "${CMAKE_SOURCE_DIR}/libs/ffmpeg/lib"
)

find_library(SWSCALE_LIBRARY 
	NAMES swscale
	HINTS "${CMAKE_SOURCE_DIR}/libs/ffmpeg/lib"
)

set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/libs/ffmpeg/include)

target_include_directories(shadowply PRIVATE ${FFMPEG_DIR})
target_link_libraries(shadowply PRIVATE ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${AVFORMAT_LIBRARY} ${SWSCALE_LIBRARY})

