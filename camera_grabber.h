#pragma once

#include <libcamera/camera.h>
#include <libcamera/framebuffer_allocator.h>

#include <memory>
#include <functional>
#include <optional>

struct CameraSettings {
    int32_t exposureTimeUs = 50000;
    float analogGain = 2;
    float brightness = 0.0;
    float contrast = 1;
    float awbRedGain = 1.5;
    float awbBlueGain = 1.5;
    float saturation = 1;
    // float digitalGain = 100;
};

class CameraGrabber {
public:
    explicit CameraGrabber(std::shared_ptr<libcamera::Camera> camera, int width, int height);
    const libcamera::StreamConfiguration &streamConfiguration();
    void setOnData(std::function<void(libcamera::Request*)> onData);
    void resetOnData();

    void startAndQueue();
    void stop();
    void requeueRequest(libcamera::Request *request);

    inline CameraSettings& GetCameraSettings() { return m_settings; }
private:
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;
    std::map<int, const char *> m_mapped;
    void requestComplete(libcamera::Request *request);

    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::CameraConfiguration> m_config;
    libcamera::FrameBufferAllocator m_buf_allocator;
    std::optional<std::function<void(libcamera::Request*)>> m_onData;

    CameraSettings m_settings;

    void setControls(libcamera::Request* request);
};
