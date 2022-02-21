#include "gl_hsv_thresholder.h"

#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>

#include <stdexcept>
#include <iostream>

#include <libdrm/drm_fourcc.h>

#include "stb_image.h"

#define GLERROR() glerror(__LINE__)
#define EGLERROR() eglerror(__LINE__)

void glerror(int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::string output;
        output.resize(128);
        snprintf(output.data(), 128, "GL error detected on line %d: 0x%04x\n", line, error);
        std::cout << output << std::endl;
        throw std::runtime_error(output);
    }
}

void eglerror(int line) {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        std::string output;
        output.resize(128);
        snprintf(output.data(), 128, "EGL error detected on line %d: 0x%04x\n", line, error);
        std::cout << output << std::endl;
        throw std::runtime_error(output);
    }
}

static constexpr const char *VERTEX_SOURCE =
        "#version 100\n"
        ""
        "attribute vec2 vertex;"
        "varying vec2 texcoord;"
        ""
        "void main(void) {"
        "   texcoord = 0.5 * (vertex + 1.0);"
        "   gl_Position = vec4(vertex, 0.0, 1.0);"
        "}";

static constexpr const char *FRAGMENT_SOURCE =
        "#version 100\n"
        "#extension GL_OES_EGL_image_external : require\n"
        ""
        "precision lowp float;"
        "precision lowp int;"
        ""
        "varying vec2 texcoord;"
        ""
        "uniform vec3 lowerThresh;"
        "uniform vec3 upperThresh;"
        "uniform samplerExternalOES tex;"
        ""
        "vec3 rgb2hsv(const vec3 p) {"
        "  const vec4 H = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);"
        // Using ternary seems to be faster than using mix and step
        "  vec4 o = mix(vec4(p.bg, H.wz), vec4(p.gb, H.xy), step(p.b, p.g));"
        "  vec4 t = mix(vec4(o.xyw, p.r), vec4(p.r, o.yzx), step(o.x, p.r));"
        ""
        "  float O = t.x - min(t.w, t.y);"
        "  const float n = 1.0e-10;"
        "  return vec3(abs(t.z + (t.w - t.y) / (6.0 * O + n)), O / (t.x + n), "
        "t.x);"
        "}"
        ""
        "bool inRange(vec3 hsv) {"
        "  const float epsilon = 0.0001;"
        "  bvec3 botBool = greaterThanEqual(hsv, lowerThresh - epsilon);"
        "  bvec3 topBool = lessThanEqual(hsv, upperThresh + epsilon);"
        "  return all(botBool) && all(topBool);"
        "}"
        ""
        "void main(void) {"
        "  vec3 col = texture2D(tex, texcoord).rgb;"
        "  gl_FragColor = vec4(col.bgr, int(inRange(rgb2hsv(col))));"
        "}";

GLuint make_shader(GLenum type, const char *source) {
    auto shader = glCreateShader(type);
    if (!shader) {
        throw std::runtime_error("failed to create shader");
    }
    glShaderSource(shader, 1, &source, nullptr);
    GLERROR();
    glCompileShader(shader);
    GLERROR();

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);

        std::string out;
        out.resize(log_size);
        glGetShaderInfoLog(shader, log_size, nullptr, out.data());

        glDeleteShader(shader);
        throw std::runtime_error("failed to compile shader with error: " + out);
    }

    return shader;
}

GLuint make_program(const char *vertex_source, const char *fragment_source) {
    auto vertex_shader = make_shader(GL_VERTEX_SHADER, vertex_source);
    auto fragment_shader = make_shader(GL_FRAGMENT_SHADER, fragment_source);

    auto program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    GLERROR();
    glAttachShader(program, fragment_shader);
    GLERROR();
    glLinkProgram(program);
    GLERROR();

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        GLint log_size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);

        std::string out;
        out.resize(log_size);
        glGetProgramInfoLog(program, log_size, nullptr, out.data());

        throw std::runtime_error("failed to link program with error: " + out);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

GlHsvThresholder::GlHsvThresholder(int width, int height, const std::vector<int>& output_buf_fds): m_width(width), m_height(height) {
    static auto glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress(
            "glEGLImageTargetTexture2DOES");
    static auto eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");

    if (!glEGLImageTargetTexture2DOES) {
        throw std::runtime_error("cannot get address of glEGLImageTargetTexture2DOES");
    }

    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLERROR();
    if (display == EGL_NO_DISPLAY) {
        throw std::runtime_error("failed to get default display");
    }

    if (!eglInitialize(display, nullptr, nullptr)) {
        throw std::runtime_error("failed to initialize display");
    }
    EGLERROR();

    const EGLint attribs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, attribs, &config, 1, &num_configs) || num_configs < 1) {
        throw std::runtime_error("failed to choose config");
    }
    EGLERROR();

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        throw std::runtime_error("failed to bind API");
    }
    EGLERROR();
    m_display = display;

    const EGLint ctx_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    auto context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (!context) {
        throw std::runtime_error("failed to create context");
    }
    EGLERROR();
    m_context = context;

    const EGLint pbuffer_attribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    auto surface = eglCreatePbufferSurface(display, config, pbuffer_attribs);
    if (!surface) {
        throw std::runtime_error("failed to create pixel buffer surface");
    }
    EGLERROR();
    m_surface = surface;

    if (!eglMakeCurrent(display, surface, surface, context)) {
        throw std::runtime_error("failed to bind egl context");
    }
    EGLERROR();

    {
        auto program = make_program(VERTEX_SOURCE, FRAGMENT_SOURCE);

        glUseProgram(program);
        GLERROR();
        glUniform1i(glGetUniformLocation(program, "tex"), 0);
        GLERROR();

        m_program = program;
    }

    for (auto fd: output_buf_fds) {
        GLuint out_tex;
        glGenTextures(1, &out_tex);
        GLERROR();
        glBindTexture(GL_TEXTURE_2D, out_tex);
        GLERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        GLERROR();

        const EGLint image_attribs[] = {
                EGL_WIDTH, static_cast<EGLint>(width),
                EGL_HEIGHT, static_cast<EGLint>(height),
                EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
                EGL_DMA_BUF_PLANE0_FD_EXT, static_cast<EGLint>(fd),
                EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
                EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(width * 4),
                EGL_NONE
        };
        auto image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, image_attribs);
        EGLERROR();
        if (!image) {
            throw std::runtime_error("failed to import fd " + std::to_string(fd));
        }

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        GLERROR();

        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        GLERROR();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        GLERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_tex, 0);
        GLERROR();

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("failed to complete framebuffer");
        }

        m_framebuffers.emplace(fd, framebuffer);
        m_renderable.push(fd);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        static GLfloat quad_varray[] = {
                -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        };

        GLuint quad_vbo;
        glGenBuffers(1, &quad_vbo);
        GLERROR();
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        GLERROR();
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_varray), quad_varray, GL_STATIC_DRAW);
        GLERROR();

        m_quad_vbo = quad_vbo;
    }
}

void GlHsvThresholder::testFrame(const std::array<GlHsvThresholder::DmaBufPlaneData, 3>& yuv_plane_data, EGLint encoding, EGLint range) {
    static auto glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress(
            "glEGLImageTargetTexture2DOES");
    static auto eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    static auto eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

    if (!glEGLImageTargetTexture2DOES) {
        throw std::runtime_error("cannot get address of glEGLImageTargetTexture2DOES");
    }

    int framebuffer_fd;
    {
        std::scoped_lock lock(m_renderable_mutex);
        if (!m_renderable.empty()) {
            framebuffer_fd = m_renderable.front();
            m_renderable.pop();
        } else {
            std::cout << "lost framebuffer, skipping" << std::endl;
            return;
        }
    }

    auto framebuffer = m_framebuffers.at(framebuffer_fd);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLERROR();

    EGLint attribs[] = {
            EGL_WIDTH, m_width,
            EGL_HEIGHT, m_height,
            EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUV420,
            EGL_DMA_BUF_PLANE0_FD_EXT, yuv_plane_data[0].fd,
            EGL_DMA_BUF_PLANE0_OFFSET_EXT, yuv_plane_data[0].offset,
            EGL_DMA_BUF_PLANE0_PITCH_EXT, yuv_plane_data[0].pitch,
            EGL_DMA_BUF_PLANE1_FD_EXT, yuv_plane_data[1].fd,
            EGL_DMA_BUF_PLANE1_OFFSET_EXT, yuv_plane_data[1].offset,
            EGL_DMA_BUF_PLANE1_PITCH_EXT, yuv_plane_data[1].pitch,
            EGL_DMA_BUF_PLANE2_FD_EXT, yuv_plane_data[2].fd,
            EGL_DMA_BUF_PLANE2_OFFSET_EXT, yuv_plane_data[2].offset,
            EGL_DMA_BUF_PLANE2_PITCH_EXT, yuv_plane_data[2].pitch,
            EGL_YUV_COLOR_SPACE_HINT_EXT, encoding,
            EGL_SAMPLE_RANGE_HINT_EXT, range,
            EGL_NONE
    };

    auto image = eglCreateImageKHR(m_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);
    EGLERROR();
    if (!image) {
        throw std::runtime_error("failed to import fd " + std::to_string(yuv_plane_data[0].fd));
    }

    GLuint texture;
    glGenTextures(1, &texture);
    GLERROR();
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    GLERROR();
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLERROR();
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    GLERROR();
    eglDestroyImageKHR(m_display, image);
    EGLERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLERROR();

    glUseProgram(m_program);
    GLERROR();

    glActiveTexture(GL_TEXTURE0);
    GLERROR();
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    GLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    GLERROR();
    // TODO: refactor these
    static auto attr_loc = glGetAttribLocation(m_program, "vertex");
    glEnableVertexAttribArray(attr_loc);
    GLERROR();
    glVertexAttribPointer(attr_loc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    GLERROR();
    static auto lll = glGetUniformLocation(m_program, "lowerThresh");
    glUniform3f(lll, 0.0, 50.0 / 255.0, 50.0 / 255.0);
    GLERROR();
    static auto uuu = glGetUniformLocation(m_program, "upperThresh");
    glUniform3f(uuu, 1.0, 1.0, 1.0);
    GLERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    GLERROR();

    glFinish();
    GLERROR();

    if(m_onComplete) {
        m_onComplete->operator()(framebuffer_fd);
    }
}

void GlHsvThresholder::setOnComplete(std::function<void(int)> onComplete) {
    m_onComplete = std::move(onComplete);
}

void GlHsvThresholder::resetOnComplete() {
    m_onComplete.reset();
}

void GlHsvThresholder::returnBuffer(int fd) {
    std::scoped_lock lock(m_renderable_mutex);
    m_renderable.push(fd);
}
