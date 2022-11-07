#pragma once

#include <EGL/egl.h>

#include <string>
#include <vector>

struct ShaderStatus {
    int gbmFd;
    struct gbm_device *gbmDevice;

    EGLDisplay display;
    EGLContext context;
};

ShaderStatus createHeadless(const std::vector<std::string>& paths);
void destroyHeadless(ShaderStatus status);
