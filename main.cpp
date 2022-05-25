#include "libcamera_jni.hpp"
#include "camera_runner.h"

int main() {
    CameraRunner *runner = (CameraRunner*)Java_org_photonvision_raspi_LibCameraJNI_createCamera(nullptr, nullptr, 1920, 1080, 30);
    if(!runner) return 1;

    runner->Start();

    return 0;

    constexpr int width = 1920, height = 1080;

    // for (int i = 0; i < 10; i++) {
        // std::cout << "Waiting for 1 second" << std::endl;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    return 0;
}
