#pragma once

#include <EGL/egl.h>

#include <string>
#include <vector>

struct HeadlessData {
    int gbmFd;
    struct gbm_device *gbmDevice;

    EGLDisplay display;
    EGLContext context;
};

HeadlessData createHeadless(const std::vector<std::string> &paths);
void destroyHeadless(HeadlessData status);
