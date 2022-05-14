/*
 * Copyright (C) 2022 Photon Vision.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "libcamera_jni.hpp" // Generated

#include "camera_manager.h"
#include <vector>

extern "C" {

#include <jni.h>

// We use jlongs like pointers, so they better be large enough
static_assert(sizeof(void *) <= sizeof(jlong));

JNIEXPORT jstring
Java_org_photonvision_raspi_LibCameraJNI_getSensorModelRaw(JNIEnv *env, jclass) {
  return env->NewStringUTF("foobar");
}

JNIEXPORT jboolean
Java_org_photonvision_raspi_LibCameraJNI_isSupported(JNIEnv *env jclass) {
    return true;
}

JNIEXPORT jlong JNICALL Java_org_photonvision_raspi_LibCameraJNI_createCamera(
    JNIEnv *env jclass, jint width, jint height, jint fps) {
  
  auto& cameras = GetAllCameraIDs();

  // Yeet all USB cameras (I hope)
	auto rem = std::remove_if(cameras.begin(), cameras.end(),
							  [](auto &cam) { return cam->id().find("/usb") != std::string::npos; });
	cameras.erase(rem, cameras.end());

	if (cameras.size() == 0) return 0;

  // Otherwise, just create the first camera left
  return (jlong) new CameraRunner(width, height, cameras[0]->id());
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_destroyCamera(JNIEnv *env jclass, jlong runnerPtr) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  delete runner;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setThresholds(
    JNIEnv *env jclass, jlong runnerPtr, jdouble hl, jdouble sl, jdouble vl, jdouble hu,
    jdouble su, jdouble vu) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetHsvThresholder().setHsvThresholds(hl, sl, vl, hu, su, vu);
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setExposure(
    JNIEnv *env jclass, jlong runnerPtr, jint exposure) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetCameraSettings().exposureTimeUs = exposure;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setBrightness(
    JNIEnv *env jclass, jlong runnerPtr, jint brightness) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetCameraSettings().brightness = brightness;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setAwbGain(
    JNIEnv *env jclass, jlong runnerPtr, jint red, jint blue) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetCameraSettings().awbRedGain = red;
  runner->GetCameraGrabber().GetCameraSettings().awbBlueGain = blue;
  return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setAnalogGain(JNIEnv *env jclass, jlong runnerPrt, jint analog) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetCameraSettings().analogGain = analog;
  return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setDigitalGain(JNIEnv *env jclass, jlong runnerPrt, jint digital) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  runner->GetCameraGrabber().GetCameraSettings().digitalGain = digital;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setRotation(
    JNIEnv *env jclass, jint rotationOrdinal) {
  int rotation = (rotationOrdinal + 3) * 90; // Degrees
  // TODO
  return true;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getFrameLatency(JNIEnv *env jclass) {
  return 0;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_awaitNewFrame(JNIEnv *env, jclass, jlong runnerPtr) {
  // TODO
  return true;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getColorFrame(JNIEnv *env jclass, jlong runnerPtr) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  return 0;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getGPUoutput(JNIEnv *env jclass, jlong runnerPtr) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  return 0;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setShouldGreyscale(JNIEnv *env jclass, jlong runnerPtr, jboolean) {
  CameraRunner *runner = (CameraRunner*)runnerPtr;
  return 0;
}

} // extern "C"
