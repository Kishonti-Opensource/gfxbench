/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import net.kishonti.swig.CLInfoCollector;
import net.kishonti.swig.CudaInfoCollector;
import net.kishonti.swig.GLInfoCollector;

import org.apache.commons.io.FileUtils;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import android.content.Context;

public class DeviceInfo {

    private AndroidInfo mAndroidInfo;
    private List<CameraInfo> mCameraInfo;

    public DeviceInfo() {
        mAndroidInfo = new AndroidInfo();
        mCameraInfo = new ArrayList<CameraInfo>();
    }

    public void collect(Context context) {

        AndroidInfoCollector aic = new AndroidInfoCollector();
        mAndroidInfo = aic.collect(context);

        CameraInfoCollector cic = new CameraInfoCollector();
        mCameraInfo = cic.collect();

        Gson gson = new Gson();
        JsonObject jroot = new JsonObject();
        jroot.add("android", gson.toJsonTree(mAndroidInfo));
        jroot.add("camera", gson.toJsonTree(mCameraInfo));

        GLInfoCollector glc = new GLInfoCollector();
        glc.collect();

        JsonObject apis = new JsonObject();
        String egls = glc.serializeEGL();
        JsonElement jegl = gson.fromJson(egls, JsonElement.class);
        apis.add("egl", jegl);

        String gless = glc.serializeGLES();
        JsonElement jgless = gson.fromJson(gless, JsonElement.class);
        apis.add("opengl_es", jgless);

        CLInfoCollector clc = new CLInfoCollector();
        clc.collect();
        String cls = clc.serialize();
        JsonElement jcl = gson.fromJson(cls, JsonElement.class);
        apis.add("opencl", jcl);

        CudaInfoCollector cuc = new CudaInfoCollector();
        cuc.collect();
        String cus = cuc.serialize();
        JsonElement jcu = gson.fromJson(cus, JsonElement.class);
        apis.add("cuda", jcu);

        jroot.add("apis", apis);

        // serialize collected info
        String json = gson.toJson(jroot);

        try {
            FileUtils.writeStringToFile(new File("/sdcard/kishonti/deviceinfo.json"), json);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

}
