#ifndef LIBCAMERA_MEME_LIBCAMERA_OPENGL_UTILITY_H
#define LIBCAMERA_MEME_LIBCAMERA_OPENGL_UTILITY_H

#include <libcamera/color_space.h>
#include <EGL/egl.h>

EGLint rangeFromColorspace(const libcamera::ColorSpace& colorSpace);
EGLint encodingFromColorspace(const libcamera::ColorSpace& colorSpace);

#endif //LIBCAMERA_MEME_LIBCAMERA_OPENGL_UTILITY_H
