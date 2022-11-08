#pragma once

#include "camera_grabber.h"
#include "concurrent_blocking_queue.h"
#include "dma_buf_alloc.h"
#include "gl_hsv_thresholder.h"
#include "libcamera_opengl_utility.h"

#include <atomic>
#include <libcamera/camera.h>
#include <memory>
#include <string>
#include <thread>

#include <opencv2/core.hpp>

// Note: destructing this class without calling `stop` if `start` was called
// is undefined behavior.
class CameraRunner {
  public:
    CameraRunner(int width, int height, int fps,
                 std::shared_ptr<libcamera::Camera> cam);
    ~CameraRunner();

    inline CameraGrabber &cameraGrabber() { return grabber; }
    inline GlHsvThresholder &thresholder() { return m_thresholder; }
    inline const std::string &model() { return m_model; }

    // Note: all following functions must be protected by mutual exclusion.
    // Failure to do so will result in UB.

    // Note: start and stop are not reenterant. Starting and stopping a camera
    // repeatedly should work, but has not been thoroughly tested.
    void start();
    void stop();

    // The memory layout of frames is as follows:
    // [ m_width * m_height thresholded frame ][ m_width * m_height * 3 color frame ]
    // the two chunks of data are directly adjacent in memory, and the unique_ptr points
    // to the start of the thresholded frame

    // Requeue a frame we have received from `outgoing`. Reusing a frame
    // intended for one camera resolution (width x height) for another
    // resolution will lead to undefined behavior. This function expects
    // exclusive access to the two `cv::Mat`s contained in the `MatPair`.
    void requeue_mat(std::unique_ptr<uint8_t[]> mat);

    // Note: this is public but is a footgun. Destructing this class while a
    // thread is blocked on this waiting for a frame is UB.
    // TODO: consider making this a shared pointer to remove this footgun
    ConcurrentBlockingQueue<std::unique_ptr<uint8_t[]>> outgoing;

  private:
    ConcurrentBlockingQueue<std::unique_ptr<uint8_t[]>> m_buffered;

    std::thread m_threshold;
    std::shared_ptr<libcamera::Camera> m_camera;
    int m_width, m_height, m_fps;

    CameraGrabber grabber;

    ConcurrentBlockingQueue<libcamera::Request *> camera_queue{};
    ConcurrentBlockingQueue<int> gpu_queue{};
    GlHsvThresholder m_thresholder;
    DmaBufAlloc allocer;

    std::vector<int> fds{};

    std::mutex camera_stop_mutex;

    std::thread threshold;
    std::thread display;

    std::string m_model;
    int32_t m_rotation = 0;
};
