#include "libcamera_jni.hpp"
#include "camera_runner.h"

int main() {
    Java_org_photonvision_raspi_LibCameraJNI_createCamera(nullptr, nullptr, 1920, 1080, 30);
    Java_org_photonvision_raspi_LibCameraJNI_startCamera(nullptr, nullptr);

    while (true);

    return 0;
}
