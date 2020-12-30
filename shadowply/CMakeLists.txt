# CMakeList.txt : CMake project for shadowply, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(PTHREAD_DIR ${CMAKE_SOURCE_DIR}/libs/pthread-win32)
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/libs/ffmpeg/include)

# Add source to this project's executable.
add_executable (shadowply "shadowply.c" "shadowply.h" "util.h" "util.c"   "window_util.h" "window_util.c" "libav.h" "libav.c" "encoder.h" "encoder.c" "runner.h" "runner.c" "capture/dc_capture.h" "capture/dc_capture.c")

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

find_library(PTHREAD_LIBRARY 
	NAMES pthread_dll
	HINTS "${PTHREAD_DIR}"
)

target_include_directories(shadowply PRIVATE ${FFMPEG_DIR} ${CMAKE_SOURCE_DIR}/libs/pthread-win32)
target_link_libraries(shadowply PRIVATE ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${AVFORMAT_LIBRARY} ${SWSCALE_LIBRARY} ${PTHREAD_LIBRARY})

