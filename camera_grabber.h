#ifndef LIBCAMERA_MEME_CAMERA_GRABBER_H
#define LIBCAMERA_MEME_CAMERA_GRABBER_H

#include <libcamera/camera.h>
#include <libcamera/framebuffer_allocator.h>

#include <memory>
#include <functional>
#include <optional>

struct CameraSettings {
    int32_t exposureTimeUs;
    float analogGain;
    float digitalGain
    float brightness;
    float contrast;
    float awbRedGain;
    float awbBlueGain;
    float saturation;
}

class CameraGrabber {
public:
    explicit CameraGrabber(std::shared_ptr<libcamera::Camera> camera, int width, int height);
    const libcamera::StreamConfiguration &streamConfiguration();
    void setOnData(std::function<void(libcamera::Request*)> onData);
    void resetOnData();

    void startAndQueue();
    void requeueRequest(libcamera::Request *request);

    CameraSettings& GetCameraSettings() { return m_settings; }

private:
    void setControls(std::function<void(libcamera::Request*)> request);

    std::vector<std::unique_ptr<libcamera::Request>> m_requests;
    std::map<int, const char *> m_mapped;
    void requestComplete(libcamera::Request *request);

    std::vector<std::pair<int, libcamera::ControlValue>> m_controlUpdatesToApply;
    CameraSettings m_settings;
    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::CameraConfiguration> m_config;
    libcamera::FrameBufferAllocator m_buf_allocator;
    std::optional<std::function<void(libcamera::Request*)>> m_onData;
};

#endif //LIBCAMERA_MEME_CAMERA_GRABBER_H
