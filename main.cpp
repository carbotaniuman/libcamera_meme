#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

#include <opencv2/core.hpp>
#include <sys/mman.h>

#include "dma_buf_alloc.h"
#include "gl_hsv_thresholder.h"
#include "camera_grabber.h"
#include "libcamera_opengl_utility.h"
#include "concurrent_blocking_queue.h"

int main() {
    constexpr int width = 1920, height = 1080;
    auto allocer = DmaBufAlloc("/dev/dma_heap/linux,cma");

    auto camera_manager = std::make_unique<libcamera::CameraManager>();
    camera_manager->start();

    auto cameras = camera_manager->cameras();
    if (cameras.size() != 1) {
        throw std::runtime_error("code expects only one camera present");
    }

    auto camera = cameras[0];
    auto grabber = CameraGrabber(std::move(camera), width, height);
    unsigned int stride = grabber.streamConfiguration().stride;

    auto camera_queue = ConcurrentBlockingQueue<libcamera::Request *>();
    grabber.setOnData([&](libcamera::Request *request) {
        camera_queue.push(request);
    });

    std::vector<int> fds {
        allocer.alloc_buf(width * height * 4),
        allocer.alloc_buf(width * height * 4),
        allocer.alloc_buf(width * height * 4)
    };

    std::thread threshold([&]() {
        auto colorspace = grabber.streamConfiguration().colorSpace.value();
        auto thresholder = GlHsvThresholder(width, height, fds);

        auto gpu_queue = ConcurrentBlockingQueue<int>();
        thresholder.setOnComplete([&](int fd) {
            gpu_queue.push(fd);
        });

        std::thread display([&]() {
            std::unordered_map<int, unsigned char *> mmaped;

            for (auto fd: fds) {
                auto mmap_ptr = mmap(nullptr, width * height * 4, PROT_READ, MAP_SHARED, fd, 0);
                if (mmap_ptr == MAP_FAILED) {
                    throw std::runtime_error("failed to mmap pointer");
                }
                mmaped.emplace(fd, static_cast<unsigned char *>(mmap_ptr));
            }

            std::array<unsigned char, width * height * 4> temp = {};
            while (true) {
                auto fd = gpu_queue.pop();
                if (fd == -1) {
                    break;
                }

                std::memcpy(&temp, mmaped.at(fd), width * height * 4);
                thresholder.returnBuffer(fd);

                cv::Mat mat(width, height, CV_8UC4, &temp);
            }
        });

        while (true) {
            auto request = camera_queue.pop();

            if (!request) {
                break;
            }

            auto planes = request->buffers().at(grabber.streamConfiguration().stream())->planes();

            std::array<GlHsvThresholder::DmaBufPlaneData, 3> yuv_data {{
             {planes[0].fd.get(),
              static_cast<EGLint>(planes[0].offset),
              static_cast<EGLint>(stride)},
             {planes[1].fd.get(),
              static_cast<EGLint>(planes[1].offset),
              static_cast<EGLint>(stride / 2)},
             {planes[2].fd.get(),
              static_cast<EGLint>(planes[2].offset),
              static_cast<EGLint>(stride / 2)},
             }};

            thresholder.testFrame(yuv_data, encodingFromColorspace(colorspace), rangeFromColorspace(colorspace));
            grabber.requeueRequest(request);
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    grabber.startAndQueue();

    for (int i = 0; i < 10; i++) {
        std::cout << "Waiting for 1 second" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
