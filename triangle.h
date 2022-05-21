#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "stb_img.h"

struct ShaderStatus {
    EGLDisplay display;
    EGLContext ctx;
}

ShaderStatus create();