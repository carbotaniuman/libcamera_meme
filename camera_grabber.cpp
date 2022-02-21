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

    auto config = m_camera->generateConfiguration({libcamera::StreamRole::VideoRecording});
    config->at(0).size.width = width;
    config->at(0).size.height = height;

    if (config->validate() == libcamera::CameraConfiguration::Invalid) {
        throw std::runtime_error("failed to validate config");
    }

    if (m_camera->configure(config.get()) < 0) {
        throw std::runtime_error("failed to configure stream");
    }

    std::cout << config->at(0).toString() << std::endl;

    auto stream = config->at(0).stream();
    if (m_buf_allocator.allocate(stream) < 0) {
        throw std::runtime_error("failed to allocate buffers");
    }
    m_config = std::move(config);

    for (const auto &buffer: m_buf_allocator.buffers(stream)) {
        auto request = m_camera->createRequest();

        auto &controls = request->controls();
        controls.set(libcamera::controls::FrameDurationLimits, {static_cast<int64_t>(8333), static_cast<int64_t>(8333)});
        controls.set(libcamera::controls::ExposureTime, 1);

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

    m_camera->requestCompleted.connect(this, &CameraGrabber::requestComplete);
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
    request->reuse(libcamera::Request::ReuseFlag::ReuseBuffers);
    if (m_camera->queueRequest(request) < 0) {
        throw std::runtime_error("failed to queue request");
    }
}

void CameraGrabber::startAndQueue() {
    if (m_camera->start()) {
        throw std::runtime_error("failed to start camera");
    }

// TODO: HANDLE THIS BETTER
    for (auto &request: m_requests) {
        if (m_camera->queueRequest(request.get()) < 0) {
            throw std::runtime_error("failed to queue request");
        }
    }
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
