public class LibCameraJNI {
    public static native String getSensorModelRaw();
    public static native boolean isSupported();
    // Get a pointer to a new runner
    public static native long createCamera(int width, int height);
    public static native boolean destroyCamera(long ptr);
    
    // Set thresholds on [0..1]
    public static native boolean setThresholds(long ptr,
        double hl, double sl, double vl,
        double hu, double su, double vu);

    // Exposure time, in microseconds
    public static native boolean setExposure(long ptr, int exposureUs);
    public static native boolean setBrightness(long ptr, double brightness);
    public static native boolean setAwbGain(long ptr, float red, float blue);
    public static native boolean setAnalogGain(long ptr, double analog);
    public static native boolean setDigitalGain(long ptr, double digital);
    public static native boolean setShouldGreyscale(long ptr, boolean shouldGreyscale);

    public static native boolean awaitNewFrame(long ptr);
    public static native long getColorFrame(long ptr);
    public static native long getGPUoutput(long ptr);
}
