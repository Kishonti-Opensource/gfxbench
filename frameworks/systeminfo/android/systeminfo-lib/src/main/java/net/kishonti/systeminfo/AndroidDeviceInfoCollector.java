/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.util.Map.Entry;

import android.content.Context;
import android.os.Build;

import net.kishonti.systeminfo.swig.*;



public class AndroidDeviceInfoCollector extends DeviceInfoCollector {
    final Context mContext;
    final Features mFeatures;
    
    public AndroidDeviceInfoCollector(Context context) {
        mContext = context;
        mFeatures = new Features(context);
    }

    @Override
    public DeviceInfo collectDeviceInfo() {
        mFeatures.run();
        
        DeviceInfo deviceInfo = new DeviceInfo();
        deviceInfo.setName(android.os.Build.MODEL);
        deviceInfo.setManufacturer(android.os.Build.BRAND);
        StringStringMap attributes = deviceInfo.getAttributes();
        for (Entry<String, String> prop: mFeatures.runGetAvaiableFeatures().entrySet()) {
            attributes.set("android/features/" + prop.getKey() + "/name", prop.getValue());
        }
        for (Entry<String, String> prop : mFeatures.getSimProps().entrySet()) {
            attributes.set(prop.getKey(), prop.getValue());
        }
        return deviceInfo;
    }



    @Override
    public OsInfo collectOsInfo() {
        OsInfo osInfo = new OsInfo();
        osInfo.setName("Android");
        osInfo.setLongName("Android");
        osInfo.setShortName("Android");
        osInfo.setFingerprint(android.os.Build.FINGERPRINT);
        osInfo.setArch(System.getProperty("os.arch"));
        osInfo.setBuild(android.os.Build.VERSION.RELEASE);

        StringStringMap attributes = osInfo.getAttributes();
        attributes.set("build_details/GLBPD_OS_BUILD_BOARD", android.os.Build.BOARD);
        attributes.set("build_details/GLBPD_OS_BUILD_BRAND", android.os.Build.BRAND);
        attributes.set("build_details/GLBPD_OS_BUILD_DEVICE", android.os.Build.DEVICE);
        attributes.set("build_details/GLBPD_OS_BUILD_DISPLAY", android.os.Build.DISPLAY);
        attributes.set("build_details/GLBPD_OS_BUILD_FINGERPRINT", android.os.Build.FINGERPRINT);
        attributes.set("build_details/GLBPD_OS_BUILD_HOST", android.os.Build.HOST);
        attributes.set("build_details/GLBPD_OS_BUILD_ID", android.os.Build.ID);
        attributes.set("build_details/GLBPD_OS_BUILD_MODEL", android.os.Build.MODEL);
        attributes.set("build_details/GLBPD_OS_BUILD_PRODUCT", android.os.Build.PRODUCT);
        attributes.set("build_details/GLBPD_OS_BUILD_TAGS", android.os.Build.TAGS);
        attributes.set("build_details/GLBPD_OS_BUILD_TYPE", android.os.Build.TYPE);
        attributes.set("build_details/GLBPD_OS_BUILD_USER", android.os.Build.USER);
        attributes.set("build_details/GLBPD_OS_BUILD_ABI", android.os.Build.CPU_ABI + "");
        attributes.set("build_details/GLBPD_OS_BUILD_ABI2", android.os.Build.CPU_ABI2 + "");
        attributes.set("build_details/GLBPD_OS_BUILD_TIME", android.os.Build.TIME + "");
        attributes.set("build_details/GLBPD_OS_BUILD_VERSION.INCREMENTAL", android.os.Build.VERSION.INCREMENTAL);
        attributes.set("build_details/GLBPD_OS_BUILD_VERSION.RELEASE", android.os.Build.VERSION.RELEASE);
        attributes.set("build_details/GLBPD_OS_BUILD_VERSION.SDK", "" + android.os.Build.VERSION.SDK_INT);
        attributes.set("build_details/GLBPD_OS_BUILD_BOOTLOADER", android.os.Build.BOOTLOADER);
        attributes.set("build_details/GLBPD_OS_BUILD_HARDWARE", android.os.Build.HARDWARE);
        attributes.set("build_details/GLBPD_OS_BUILD_MANUFACTURER", android.os.Build.MANUFACTURER);
        attributes.set("build_details/GLBPD_OS_BUILD_SERIAL", android.os.Build.SERIAL);
        
        for (Entry<Object, Object> property: System.getProperties().entrySet()) {
            attributes.set("system_properties/" + property.getKey().toString(), property.getValue().toString());
        }
        for (Entry<String, String> var: System.getenv().entrySet()) {
            attributes.set("system_env/" + var.getKey(), var.getValue());
        }
        for (Entry<String, String> prop: mFeatures.runGetprop().entrySet()) {
            attributes.set("env/" + prop.getKey(), prop.getValue());
        }
        return osInfo;
    }



    @Override
    public DisplayInfoVector collectDisplayInfo() {
        Display display = new Display(mContext);
        display.run();
        
        DisplayInfo displayInfo = new DisplayInfo();
        displayInfo.setName("Android display");
        displayInfo.setDiagonalInches(display.screenInches);
        displayInfo.setWidthPixels(display.xResolution);
        displayInfo.setHeightPixels(display.yResolution);
        displayInfo.setXDpi(display.xDpi);
        displayInfo.setYDpi(display.yDpi);
        
        StringStringMap attributes = displayInfo.getAttributes();
        for (Entry<String, Boolean> feature: display.getFeatures().entrySet()) {
            attributes.set("features/" + feature.getKey(), feature.getValue().toString());
        }
        
        DisplayInfoVector displayInfoVector = new DisplayInfoVector();
        displayInfoVector.add(displayInfo);
        return displayInfoVector;
    }



    @Override
    public CpuInfoVector collectCpuInfo() {
        CPU cpu = new CPU();
        cpu.run();
        
        CpuInfo cpuInfo = new CpuInfo();
        cpuInfo.setCores(cpu.getCores());
        if (cpu.getFrequency() != null && cpu.getFrequency().size() > 0) {
            cpuInfo.setFrequencyMHz(cpu.getFrequency().get(0));
        }
        StringStringMap attributes = cpuInfo.getAttributes();
        for (Entry<String, String> file : cpu.getCPUDataFiles().entrySet()) {
            attributes.set(file.getKey().substring(1).replaceAll("/", "_"), file.getValue());
        }
        
        CpuInfoVector cpuInfoVector = new CpuInfoVector();
        cpuInfoVector.add(cpuInfo);
        return cpuInfoVector;
    }



    @Override
    public GpuInfoVector collectGpuInfo() {
        GPU gpu = new GPU();
        gpu.run();
        
        GpuInfo gpuInfo = new GpuInfo();
        gpuInfo.setName(gpu.getData("GL_VENDOR") + " " + gpu.getData("GL_RENDERER"));
        gpuInfo.setIds(gpu.getData("GL_VERSION"));
        GpuInfoVector gpuInfoVector = new GpuInfoVector();
        gpuInfoVector.add(gpuInfo);
        return gpuInfoVector;
    }



    @Override
    public MultiGpuInfo collectMultiGpuInfo() {
        return new MultiGpuInfo();
    }



    @Override
    public MemoryInfo collectMemoryInfo() {
        Memory memory = new Memory();
        memory.run();
        
        MemoryInfo memoryInfo = new MemoryInfo();
        memoryInfo.setSizeBytes(memory.getMemory());
        return memoryInfo;
    }



    @Override
    public StorageInfoVector collectStorageInfo() {
        Memory memory = new Memory();
        memory.run();
        
        StorageInfoVector storageInfoVector = new StorageInfoVector();
        for (int i = 0; i < memory.getNumOfStorage(); i++) {
            Object[] data = memory.getData(i);
            if (data == null) {
                continue; // FIXME: is this really necessary?
            }
            StorageInfo storageInfo = new StorageInfo();
            storageInfo.setName((String)data[0]);
            storageInfo.setIsRemovable((Boolean)data[1]);
            storageInfo.setSizeBytes((Long)data[2]);
            storageInfoVector.add(storageInfo);
        }
        return storageInfoVector;
    }



    @Override
    public BatteryInfoVector collectBatteryInfo() {
        BatteryInfo batteryInfo = new BatteryInfo();
        try {
            Battery.getInstance().register(mContext);
            batteryInfo.setIsCharging(Battery.getInstance().getCharging());
            batteryInfo.setIsConnected(Battery.getInstance().getConnected());
            batteryInfo.setLevelRatio(Battery.getInstance().getBatteryLevel());
        } finally {
            Battery.getInstance().release();
        }
        
        BatteryInfoVector batteryInfoVector = new BatteryInfoVector();
        batteryInfoVector.add(batteryInfo);
        return batteryInfoVector;
    }



    @Override
    public CameraInfoVector collectCameraInfo() {
        Camera camera = new Camera(mContext);
        camera.run();

        CameraInfoVector cameraInfoVector = new CameraInfoVector();
        for (int i = 0; i < camera.getNumberOfCameras(); i++) {
            Camera.CameraProps cameraProps = camera.getCamera(i);
            if (cameraProps == null) {
                continue; // FIXME: is this really necessary?
            }
            
            CameraInfo cameraInfo = new CameraInfo();
            cameraInfo.setType(cameraProps.isFrontFacing ? "CAMERA_TYPE_FRONT" : "CAMERA_TYPE_BACK");
            cameraInfo.setPictureWidthPixels(cameraProps.maxPicX);
            cameraInfo.setPictureHeightPixels(cameraProps.maxPicY);
            cameraInfo.setPictureResolutionMP(cameraProps.maxPicX * cameraProps.maxPicY * 0.000001);
            cameraInfo.setVideoWidthPixels(cameraProps.maxVidX);
            cameraInfo.setVideoHeightPixels(cameraProps.maxVidY);
            cameraInfo.setVideoResolutionMP(cameraProps.maxVidX * cameraProps.maxVidY * 0.000001);
            cameraInfo.setHasAutofocus(cameraProps.hasAutoFocus);
            cameraInfo.setHasFlash(cameraProps.flashSupported);
            cameraInfo.setHasFaceDetection(cameraProps.faceDetection);
            cameraInfo.setHasHdr(cameraProps.hdrSupported);
            cameraInfo.setHasTouchFocus(cameraProps.touchFocus);
            cameraInfo.setFlat(cameraProps.flatCameraInfo);
            cameraInfoVector.add(cameraInfo);
        }
        return cameraInfoVector;
    }



    @Override
    public FeatureInfo collectFeatureInfo() {
        FeatureInfo featureInfo = new FeatureInfo();
        StringBoolMap features = featureInfo.getFeatures();
        features.set("wifi", mFeatures.haveWifi());
        features.set("gps", mFeatures.haveGPS());
        features.set("bluetooth", mFeatures.haveBlueTooth());
        features.set("nfc", mFeatures.haveNFC());
        features.set("camera (rear)", mFeatures.haveBackCamera());
        features.set("camera (face)", mFeatures.haveFrontCamera());
        features.set("simcards", mFeatures.haveSimCards());
        features.set("accelerometer", mFeatures.haveAccelerometer());
        features.set("barometer", mFeatures.haveBarometer());
        features.set("gyroscope", mFeatures.haveGyroscope());
        features.set("compass", mFeatures.haveCompass());
        features.set("proximity", mFeatures.haveProximity());
        features.set("lightsensor", mFeatures.haveLightsensor());
        return featureInfo;
    }
    
    
    
    @Override
    public SensorInfo collectSensorInfo() {
        SensorInfo sensorInfo = new SensorInfo();
        StringStringMap sensors = sensorInfo.getSensors();
        for (Entry<String, String[]> prop : mFeatures.getSensors().entrySet()) {
            sensors.set(prop.getValue()[0], prop.getValue()[1]);
        }
        return sensorInfo;
    }
}
