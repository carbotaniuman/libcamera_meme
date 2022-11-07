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
#include "camera_runner.h"

static CameraRunner *runner = NULL;

extern "C" {

#include <jni.h>

// We use jlongs like pointers, so they better be large enough
static_assert(sizeof(void *) <= sizeof(jlong));

JNIEXPORT jstring
Java_org_photonvision_raspi_LibCameraJNI_getSensorModelRaw(JNIEnv *env, jclass) {
  if(runner)
    return env->NewStringUTF(runner->model().c_str());
  else 
    return env->NewStringUTF("");
}

JNIEXPORT jboolean
Java_org_photonvision_raspi_LibCameraJNI_isSupported(JNIEnv *env, jclass) {
    return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_createCamera(
    JNIEnv *env, jclass, jint width, jint height, jint fps) {
  
  std::vector<std::shared_ptr<libcamera::Camera>> cameras = GetAllCameraIDs();

  // Yeet all USB cameras (I hope)
	auto rem = std::remove_if(cameras.begin(), cameras.end(),
							  [](auto &cam) { return cam->id().find("/usb") != std::string::npos; });
	cameras.erase(rem, cameras.end());

	if (cameras.size() == 0) return 0;

  // // Otherwise, just create the first camera left
  runner = new CameraRunner(width, height, fps, cameras[0]);
  return runner != 0;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_startCamera(
    JNIEnv *, jclass) {
  if (runner) {
    runner->start();
    return true;
  }      
  return false;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_destroyCamera(JNIEnv *env, jclass) {
  if(!runner) return false;

    runner->stop();
    delete runner;
    return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setThresholds(
    JNIEnv *env, jclass, jdouble hl, jdouble sl, jdouble vl, jdouble hu,
    jdouble su, jdouble vu) {
  if(!runner) return false;
  runner->getThresholder().setHsvThresholds(hl, sl, vl, hu, su, vu);
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setExposure(
    JNIEnv *env, jclass, jint exposure) {
  if(!runner) return false;
  runner->getCameraGrabber().GetCameraSettings().exposureTimeUs = exposure;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setBrightness(
    JNIEnv *env, jclass, jdouble brightness) {
  if(!runner) return false;
  runner->getCameraGrabber().GetCameraSettings().brightness = brightness;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setAwbGain(
    JNIEnv *env, jclass, jdouble red, jdouble blue) {
  if(!runner) return false;
  runner->getCameraGrabber().GetCameraSettings().awbRedGain = red;
  runner->getCameraGrabber().GetCameraSettings().awbBlueGain = blue;
  return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setAnalogGain(JNIEnv *env, jclass, jdouble analog) {
  if(!runner) return false;
  runner->getCameraGrabber().GetCameraSettings().analogGain = analog;
  return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setDigitalGain(JNIEnv *env, jclass, jdouble digital) {
  if(!runner) return false;
  // runner->getCameraGrabber().GetCameraSettings().digitalGain = digital;
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setRotation(
    JNIEnv *env, jclass, jint rotationOrdinal) {
  // int rotation = (rotationOrdinal + 3) * 90; // Degrees
  // TODO
  return true;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getFrameLatency(JNIEnv *env, jclass) {
  return 0;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_awaitNewFrame(JNIEnv *env, jclass) {
  // TODO
  return true;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getColorFrame(JNIEnv *env, jclass) {
  if(!runner) return false;
  return 0;
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getGPUoutput(JNIEnv *env, jclass) {
  if(!runner) return false;
  return 0;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setShouldGreyscale(JNIEnv *env, jclass, jboolean) {
  if(!runner) return false;
  return 0;
}

} // extern "C"
