
/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_photonvision_raspi_LibCameraJNI */

#ifndef _Included_org_photonvision_raspi_LibCameraJNI
#define _Included_org_photonvision_raspi_LibCameraJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    getSensorModelRaw
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getSensorModelRaw(JNIEnv *, jclass);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    isVCSMSupported
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_isSupported(JNIEnv *, jclass);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    createCamera
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_createCamera(
    JNIEnv *, jclass, jint, jint, jint);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    destroyCamera
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_destroyCamera(JNIEnv *, jclass);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setThresholds
 * Signature: (DDDDDD)V
 */
JNIEXPORT void JNICALL Java_org_photonvision_raspi_LibCameraJNI_setThresholds(
    JNIEnv *, jclass, jdouble, jdouble, jdouble, jdouble, jdouble, jdouble);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setExposure
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setExposure(JNIEnv *, jclass, jint);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setBrightness
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setBrightness(JNIEnv *, jclass, jint);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setGain
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setGain(JNIEnv *, jclass, jint);

JNIEXPORT jboolean JNICALL Java_org_photonvision_raspi_LibCameraJNI_setAwbGain(
    JNIEnv *, jclass, jint red, jint blue);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setRotation
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_photonvision_raspi_LibCameraJNI_setRotation(JNIEnv *, jclass, jint);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    setShouldCopyColor
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_org_photonvision_raspi_LibCameraJNI_setShouldCopyColor(
    JNIEnv *, jclass, jboolean);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    getFrameLatency
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_getFrameLatency(JNIEnv *, jclass);

/*
 * Class:     org_photonvision_raspi_LibCameraJNI
 * Method:    grabFrame
 * Signature: (Z)J
 */
JNIEXPORT jlong JNICALL
Java_org_photonvision_raspi_LibCameraJNI_grabFrame(JNIEnv *, jclass, jboolean);

#ifdef __cplusplus
}
#endif
#endif