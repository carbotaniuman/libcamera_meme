#pragma once

#include <vector>
#include <string>
#include <memory>
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

std::vector<std::shared_ptr<libcamera::Camera>> GetAllCameraIDs();
