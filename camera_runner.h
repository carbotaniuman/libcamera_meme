#pragma once

#include <string>
#include "camera_grabber.h"
#include <thread>
#include <libcamera/camera.h>
#include "gl_hsv_thresholder.h"
#include "camera_grabber.h"
#include "libcamera_opengl_utility.h"
#include "concurrent_blocking_queue.h"
#include "dma_buf_alloc.h"

class CameraRunner {
public:
  CameraRunner(int width, int height, int fps, const std::shared_ptr<libcamera::Camera> cam);


  CameraGrabber GetCameraGrabber();

  void Start();
  void Stop();

private:
  std::thread m_threshold;
  std::shared_ptr<libcamera::Camera> m_camera;
  int m_width, m_height, m_fps;

  CameraGrabber grabber;
  ConcurrentBlockingQueue<libcamera::Request *> camera_queue {};
  ConcurrentBlockingQueue<int> gpu_queue {};
  GlHsvThresholder thresholder;
  DmaBufAlloc allocer;

  std::vector<int> fds {};

  std::thread threshold;
  std::thread display;

  bool threshold_thread_run = true;
  bool display_thread_run = true;
};