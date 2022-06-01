#pragma once

#include <EGL/egl.h>

#include <string>

struct ShaderStatus {
    int gbmFd;
    struct gbm_device *gbmDevice;

    EGLDisplay display;
    EGLContext context;
};

ShaderStatus createHeadless(const std::string& name);
