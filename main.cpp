#include "libcamera_jni.hpp"
#include "camera_runner.h"

int main() {
    CameraRunner *runner = (CameraRunner*)Java_org_photonvision_raspi_LibCameraJNI_createCamera(nullptr, nullptr, 1920, 1080, 30);
    if(!runner) return 1;

    runner->Start();

    while (true);

    return 0;
}
