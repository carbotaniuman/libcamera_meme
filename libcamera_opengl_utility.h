#pragma once

#include <libcamera/color_space.h>
#include <EGL/egl.h>

EGLint rangeFromColorspace(const libcamera::ColorSpace& colorSpace);
EGLint encodingFromColorspace(const libcamera::ColorSpace& colorSpace);
