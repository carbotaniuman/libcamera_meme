#include "camera_runner.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <sys/mman.h>

using namespace std::chrono;
using namespace std::chrono_literals;

static double approxRollingAverage (double avg, double new_sample) {
    avg -= avg / 50;
    avg += new_sample / 50;

    return avg;
}


CameraRunner::CameraRunner(int width, int height, int fps, const std::shared_ptr<libcamera::Camera> cam) 
    : m_camera(cam), m_width(width), m_height(height), m_fps(fps),
        grabber(m_camera, m_width, m_height),
        thresholder(m_width, m_height),
        allocer("/dev/dma_heap/linux,cma")
    {

    grabber.setOnData([&](libcamera::Request *request) {
        camera_queue.push(request);
    });

    fds = {
            allocer.alloc_buf(m_width * m_height * 4),
            allocer.alloc_buf(m_width * m_height * 4),
            allocer.alloc_buf(m_width * m_height * 4)
    };
}

void CameraRunner::Start() {
    unsigned int stride = grabber.streamConfiguration().stride;
    
    threshold = std::thread([&]() {
        thresholder.start(fds);
        auto colorspace = grabber.streamConfiguration().colorSpace.value();

        thresholder.setOnComplete([&](int fd) {
            gpu_queue.push(fd);
        });

        double gpuTimeAvgMs = 0;

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

            auto begintime = steady_clock::now();
            thresholder.testFrame(yuv_data, encodingFromColorspace(colorspace), rangeFromColorspace(colorspace));

            std::chrono::duration<double, std::milli> elapsedMillis = steady_clock::now() - begintime;
            if (elapsedMillis > 0.9ms) {
                gpuTimeAvgMs = approxRollingAverage(gpuTimeAvgMs, elapsedMillis.count());
                std::cout << "GLProcess: " << gpuTimeAvgMs << std::endl;
            }
            grabber.requeueRequest(request);
        }
    });

    display = std::thread([&]() {
        std::unordered_map<int, unsigned char *> mmaped;

        for (auto fd: fds) {
            auto mmap_ptr = mmap(nullptr, m_width * m_height * 4, PROT_READ, MAP_SHARED, fd, 0);
            if (mmap_ptr == MAP_FAILED) {
                throw std::runtime_error("failed to mmap pointer");
            }
            mmaped.emplace(fd, static_cast<unsigned char *>(mmap_ptr));
        }

        cv::Mat threshold_mat(m_height, m_width, CV_8UC1);
        unsigned char *threshold_out_buf = threshold_mat.data;
        cv::Mat color_mat(m_height, m_width, CV_8UC3);
        unsigned char *color_out_buf = color_mat.data;

        double copyTimeAvgMs = 0;

        while (true) {
            auto fd = gpu_queue.pop();
            if (fd == -1) {
                break;
            }

            auto begintime = steady_clock::now();
            auto input_ptr = mmaped.at(fd);
            int bound = m_width * m_height;

            for (int i = 0; i < bound; i++) {
                std::memcpy(color_out_buf + i * 3, input_ptr + i * 4, 3);
                threshold_out_buf[i] = input_ptr[i * 4 + 3];
            }

            // pls don't optimize these writes out compiler
            std::cout << reinterpret_cast<uint64_t>(threshold_out_buf) << " " << reinterpret_cast<uint64_t>(color_out_buf) << std::endl;

            thresholder.returnBuffer(fd);

            std::chrono::duration<double, std::milli> elapsedMillis = steady_clock::now() - begintime;
            // if (elapsedMillis > 0.9ms) {
                copyTimeAvgMs = approxRollingAverage(copyTimeAvgMs, elapsedMillis.count());
                std::cout << "Copy: " << copyTimeAvgMs << std::endl;
            // }

            // cv::imshow("cam_color", color_mat);
            // cv::imshow("cam_single", threshold_mat);
            // cv::waitKey(1);
            static int i = 0;
            i++;
            static char arr[50];
            snprintf(arr,sizeof(arr),"color_%i.png", i);
            cv::imwrite(arr, color_mat);
            snprintf(arr,sizeof(arr),"thresh_%i.png", i);
            cv::imwrite(arr, threshold_mat);
        }
    });

    // TODO don't think we need this
    std::this_thread::sleep_for(std::chrono::seconds(2));

    grabber.startAndQueue();
}