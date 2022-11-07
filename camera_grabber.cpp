#include "camera_grabber.h"

#include <iostream>
#include <stdexcept>

#include <libcamera/control_ids.h>
#include <sys/mman.h>

CameraGrabber::CameraGrabber(std::shared_ptr<libcamera::Camera> camera, int width, int height) : m_camera(std::move(camera)),
                                                                                                 m_buf_allocator(m_camera) {
    if (m_camera->acquire()) {
        throw std::runtime_error("failed to acquire camera");
    }

    std::cout <<"A" << std::endl;

    auto config = m_camera->generateConfiguration({libcamera::StreamRole::VideoRecording});
    config->at(0).size.width = width;
    config->at(0).size.height = height;
    config->transform = libcamera::Transform::Identity;

    std::cout <<"B" << std::endl;

    if (config->validate() == libcamera::CameraConfiguration::Invalid) {
        throw std::runtime_error("failed to validate config");
    }

    std::cout <<"C" << std::endl;

    if (m_camera->configure(config.get()) < 0) {
        throw std::runtime_error("failed to configure stream");
    }

    std::cout <<"D" << std::endl;

    std::cout << config->at(0).toString() << std::endl;

    auto stream = config->at(0).stream();
    if (m_buf_allocator.allocate(stream) < 0) {
        throw std::runtime_error("failed to allocate buffers");
    }
    m_config = std::move(config);

    std::cout <<"E" << std::endl;

    for (const auto &buffer: m_buf_allocator.buffers(stream)) {
        auto request = m_camera->createRequest();

        auto &controls = request->controls();
        // controls.set(libcamera::controls::FrameDurationLimits, {static_cast<int64_t>(8333), static_cast<int64_t>(8333)});
        // controls.set(libcamera::controls::ExposureTime, 10000);

        request->addBuffer(stream, buffer.get());
        m_requests.push_back(std::move(request));

        size_t len = 0;
        int fd = 0;
        for (const auto &plane: buffer->planes()) {
            fd = plane.fd.get();
            len += plane.length;
        }
        auto memory = static_cast<const char *>(mmap(nullptr, len, PROT_READ, MAP_SHARED, fd, 0));
        m_mapped.emplace(fd, memory);
    }

    std::cout << "F" << std::endl;

    m_camera->requestCompleted.connect(this, &CameraGrabber::requestComplete);
}

CameraGrabber::~CameraGrabber() {
    m_camera->release();
}

void CameraGrabber::requestComplete(libcamera::Request *request) {
    if (request->status() == libcamera::Request::RequestCancelled) {
        return;
    }

    static int i = 0;

    i++;

    if(m_onData) {
        m_onData->operator()(request);
    }

    std::cout << i << std::endl;
}

void CameraGrabber::requeueRequest(libcamera::Request *request) {
    // This resets all our controls
    // https://github.com/kbingham/libcamera/blob/master/src/libcamera/request.cpp#L397
    request->reuse(libcamera::Request::ReuseFlag::ReuseBuffers);

    setControls(request);

    if (m_camera->queueRequest(request) < 0) {
        throw std::runtime_error("failed to queue request");
    }
}

void CameraGrabber::setControls(libcamera::Request *request) {

    auto &controls = request->controls();
    controls.set(libcamera::controls::AeEnable, false); // Auto exposure disabled
    controls.set(libcamera::controls::AwbEnable, false); // AWB disabled
    controls.set(libcamera::controls::ExposureTime, m_settings.exposureTimeUs); // in microseconds
    controls.set(libcamera::controls::AnalogueGain, m_settings.analogGain); // Analog gain, min 1 max big number?
    controls.set(libcamera::controls::ColourGains, libcamera::Span<const float, 2>{{ m_settings.awbRedGain, m_settings.awbBlueGain }}); // AWB gains, red and blue, unknown range
    controls.set(libcamera::controls::Brightness, m_settings.brightness); // -1 to 1, 0 means unchanged
    controls.set(libcamera::controls::Contrast, m_settings.contrast); // Nominal 1
    controls.set(libcamera::controls::Saturation, m_settings.saturation); // Nominal 1, 0 would be greyscale
    controls.set(libcamera::controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>{{m_settings.exposureTimeUs, m_settings.exposureTimeUs}}); // Set default to zero, we have specified the exposure time

    // Additionally, we can set crop regions and stuff
    // controls.set(libcamera::controls::DigitalGain, m_settings.digitalGain); // Digital gain, unknown range
}

void CameraGrabber::startAndQueue() {
    if (m_camera->start()) {
        throw std::runtime_error("failed to start camera");
    }

// TODO: HANDLE THIS BETTER
    for (auto &request: m_requests) {
        setControls(request.get());
        if (m_camera->queueRequest(request.get()) < 0) {
            throw std::runtime_error("failed to queue request");
        }
    }
}

void CameraGrabber::stop() {
    m_camera->stop();
}

void CameraGrabber::setOnData(std::function<void(libcamera::Request *)> onData) {
    m_onData = std::move(onData);
}

void CameraGrabber::resetOnData() {
    m_onData.reset();
}

const libcamera::StreamConfiguration &CameraGrabber::streamConfiguration() {
    return m_config->at(0);
}
