#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <sys/mman.h>

#include "dma_buf_alloc.h"
#include "gl_hsv_thresholder.h"
#include "camera_grabber.h"
#include "libcamera_opengl_utility.h"
#include "concurrent_blocking_queue.h"

class CameraRunner {
public:
  CameraRunner(int width, int height, int fps, const std::string &id);

  CameraGrabber GetCameraGrabber();

  void Stop();

private:
  std::thread m_threshold;

  bool threshold_thread_run = true;
  bool display_thread_run = true;
}

static std::vector<std::string> GetAllCameraIDs();