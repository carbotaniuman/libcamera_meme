#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ShaderStatus {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
};

int create(struct ShaderStatus *status);

#ifdef __cplusplus
}
#endif
