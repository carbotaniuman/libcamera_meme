cmake_minimum_required(VERSION 3.16)
project(libcamera_meme)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
find_package(OpenCV REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

add_executable(libcamera_meme main.cpp concurrent_blocking_queue.h camera_grabber.cpp dma_buf_alloc.cpp gl_hsv_thresholder.cpp libcamera_opengl_utility.cpp)
target_include_directories(libcamera_meme PUBLIC ${OPENGL_INCLUDE_DIRS} ${LIBDRM_INCLUDE_DIRS} ${LIBCAMERA_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(libcamera_meme PUBLIC OpenGL::GL OpenGL::EGL Threads::Threads ${LIBCAMERA_LINK_LIBRARIES} ${OpenCV_LIBS})
