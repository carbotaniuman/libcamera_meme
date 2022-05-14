#include "camera_manager.h"

CameraRunner::CameraRunner(int width, int height, const std::string& id) {
    auto cameras = camera_manager_->cameras();
    auto *camera = std::find_if(cameras.begin(), cameras.end(), [](auto &cam) { return cam->id() == id; })

    auto allocer = DmaBufAlloc("/dev/dma_heap/linux,cma");

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


    m_threshold ([&]() {
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

            cv::Mat threshold_mat(height, width, CV_8UC1);
            unsigned char *threshold_out_buf = threshold_mat.data;
            cv::Mat color_mat(height, width, CV_8UC3);
            unsigned char *color_out_buf = color_mat.data;

            while (display_thread_run) {
                auto fd = gpu_queue.pop();
                if (fd == -1) {
                    break;
                }

                auto input_ptr = mmaped.at(fd);
                int bound = width * height;

                for (int i = 0; i < bound; i++) {
                    std::memcpy(color_out_buf + i * 3, input_ptr + i * 4, 3);
                    threshold_out_buf[i] = input_ptr[i * 4 + 3];
                }

                // pls don't optimize these writes out compiler
                std::cout << reinterpret_cast<uint64_t>(threshold_out_buf) << " " << reinterpret_cast<uint64_t>(color_out_buf) << std::endl;

                thresholder.returnBuffer(fd);
                // cv::imshow("cam", mat);
                // cv::waitKey(3);
            }
        });

        while (threshold_thread_run) {
            auto request = camera_queue.pop();

            if (!request) {
                break;
            }

            // in Nanoseconds, from the Linux CLOCK_BOOTTIME 
            auto sensorTimestamp = request->controls()->get(libcamera::controls::SensorTimestamp); 

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

        // need to stop above thread too
        display_thread_run = false;
        display.join();
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    grabber.startAndQueue();
}

void CameraRunner::Stop() {
  threshold_thread_run = false;
  m_threshold.join();
}

static std::vector<std::string> GetAllCameraIDs() {
  static libcamera::CameraManager* camera_manager;
  if(!camera_manager) {
    camera_manager = std::make_unique<libcamera::CameraManager>();
    camera_manager->start();
  }
  return camera_manager->cameras();
}