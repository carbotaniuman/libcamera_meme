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

extern "C" {

#include <jni.h>

// We use jlongs like pointers, so they better be large enough
static_assert(sizeof(void *) <= sizeof(jlong));

JNIEXPORT jstring
Java_org_photonvision_raspi_LibCameraJNI_getSensorModelRaw(JNIEnv *env, jclass) {
  return env->NewStringUTF("foobar");
}

JNIEXPORT jboolean
Java_org_photonvision_raspi_LibCameraJNI_isSupported(JNIEnv *, jclass) {
    return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_createCamera(
    JNIEnv *, jclass, jint width, jint height, jint fps) {
    return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_destroyCamera(JNIEnv *, jclass) {
  return true;
}

JNIEXPORT void JNICALL Java_org_photonvision_raspi_LibCameraJNI_setThresholds(
    JNIEnv *, jclass, jdouble h_l, jdouble s_l, jdouble v_l, jdouble h_u,
    jdouble s_u, jdouble v_u) {
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setExposure(
    JNIEnv *, jclass, jint exposure) {
  constexpr int padding_microseconds = 1000;

  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setBrightness(
    JNIEnv *, jclass, jint brightness) {
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setAwbGain(
    JNIEnv *, jclass, jint red, jint blue) {
  return true;
}

JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setGain(JNIEnv *, jclass, jint gain) {
  return true;
}

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setRotation(
    JNIEnv *, jclass, jint rotationOrdinal) {
  int rotation = (rotationOrdinal + 3) * 90; // Degrees
  return true;
}

JNIEXPORT void JNICALL Java_org_photonvision_raspi_LibCameraJNI_setShouldCopyColor(
    JNIEnv *, jclass, jboolean should_copy_color) {
}

JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getFrameLatency(JNIEnv *, jclass) {
  return 0;
}

JNIEXPORT jlong JNICALL Java_org_photonvision_raspi_LibCameraJNI_grabFrame(
    JNIEnv *, jclass, jboolean should_return_color) {
  return 0;
}

} // extern "C"
