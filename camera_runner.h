#pragma once

#include <string>
#include "camera_grabber.h"
#include <thread>
#include <libcamera/camera.h>

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

  bool threshold_thread_run = true;
  bool display_thread_run = true;
};
