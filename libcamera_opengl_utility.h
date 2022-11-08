#pragma once

#include <EGL/egl.h>
#include <libcamera/color_space.h>

EGLint rangeFromColorspace(const libcamera::ColorSpace &colorSpace);
EGLint encodingFromColorspace(const libcamera::ColorSpace &colorSpace);
