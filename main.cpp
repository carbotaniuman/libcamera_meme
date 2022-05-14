#include "camera_manager.h"
#include <iostream>

int main() {
    constexpr int width = 1920, height = 1080;

    std::cout << "Num cameras: " << GetAllCameraIDs().size() << std::endl;

    CameraRunner runner{640, 480, GetAllCameraIDs[0]};

    for (int i = 0; i < 10; i++) {
        std::cout << "Waiting for 1 second" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    runner.Stop();

    return 0;
}
