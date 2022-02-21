#ifndef LIBCAMERA_MEME_GL_HSV_THRESHOLDER_H
#define LIBCAMERA_MEME_GL_HSV_THRESHOLDER_H

#include <array>
#include <functional>
#include <string>
#include <queue>
#include <utility>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

class GlHsvThresholder {
public:
    struct DmaBufPlaneData {
        int fd;
        EGLint offset;
        EGLint pitch;
    };

    explicit GlHsvThresholder(int width, int height, const std::vector<int>& output_buf_fds);
    void setOnComplete(std::function<void(int)> onComplete);
    void resetOnComplete();

    void returnBuffer(int fd);
    void testFrame(const std::array<GlHsvThresholder::DmaBufPlaneData, 3>& yuv_plane_data, EGLint encoding, EGLint range);
private:
    int m_width;
    int m_height;
    std::optional<std::function<void(int)>> m_onComplete;

    EGLDisplay m_display;
    EGLContext m_context;
    EGLSurface m_surface;

    std::unordered_map<int, GLuint> m_framebuffers; // (dma_buf fd, framebuffer)
    std::queue<int> m_renderable;
    std::mutex m_renderable_mutex;

    GLuint m_quad_vbo;
    GLuint m_program;
};

#endif //LIBCAMERA_MEME_GL_HSV_THRESHOLDER_H
