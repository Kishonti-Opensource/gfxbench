/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.util.List;

import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Camera.Parameters;
import android.os.Build;

public class Camera extends InfoProvider implements Runnable {
	private int mNumberOfCameras;
	private CameraProps[] mCameras;

	private boolean FEATURE_CAMERA;
	private boolean FEATURE_CAMERA_FRONT;

	public Camera(Context context) {
		
		super(context);
	}

	public class CameraProps {
		public boolean isFrontFacing;
		public boolean faceDetection;
		public boolean touchFocus;
		public boolean hdrSupported;
		public boolean hasAutoFocus;
		public boolean flashSupported;
		public boolean picSupported;
		public int maxPicX;
		public int maxPicY;
		public boolean vidSupported;
		public int maxVidX;
		public int maxVidY;
		public String flatCameraInfo = "";
	}

	public int getNumberOfCameras() {
		
		return mNumberOfCameras;
	}

	public CameraProps getCamera(int index) {
		
		if (mCameras != null) {

			return mCameras[index];
		}
		return null;
	}

	@Override
	public void run() {
		// the FEATURE_CAMERA probably changed in history
		// there is a 'new' FEATURE_CAMERA_ANY, it may happen that FEATURE_CAMERA has reports back camera since?
		// TODO: camera hardware check on a tablet with front camera only to prove it
		if (context != null) {
			FEATURE_CAMERA = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA);
			FEATURE_CAMERA_FRONT = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FRONT);
		}

		if (!FEATURE_CAMERA && !FEATURE_CAMERA_FRONT) {
			return;
		}

        try {
            mNumberOfCameras = android.hardware.Camera.getNumberOfCameras();
        } catch (Exception e) {
            mNumberOfCameras = 0;
            return;
        }

		int res = context.checkCallingOrSelfPermission("android.permission.CAMERA");
		if (res == PackageManager.PERMISSION_GRANTED) {

			mCameras = new CameraProps[mNumberOfCameras];
			for (int i = 0; i < mNumberOfCameras; i++) {
				CameraProps thisCameraInfo = new CameraProps();

				android.hardware.Camera c = null;
				try {
					c = android.hardware.Camera.open(i);
					Parameters p = c.getParameters();// TODO slow this one
					android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
					android.hardware.Camera.getCameraInfo(i, info);

					thisCameraInfo.isFrontFacing = (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT);

					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
						thisCameraInfo.faceDetection = (getSupportedFaces(p) > 0);
					}

					thisCameraInfo.touchFocus = (p.getMaxNumFocusAreas() > 0);

					List<String> sceneModes = p.getSupportedSceneModes();
					for (int j = 0; (sceneModes != null) && (j < sceneModes.size()); j++) {
						if (sceneModes.get(j).equals("hdr")) {
							thisCameraInfo.hdrSupported = true;
						}
					}

					List<String> focusModes = p.getSupportedFocusModes();
					for (int j = 0; (focusModes != null) && (j < focusModes.size()); j++) {
						if (focusModes.get(j).equals("auto")) {
							thisCameraInfo.hasAutoFocus = true;
						}
					}

					List<String> flashModes = p.getSupportedFlashModes();
					for (int j = 0; (flashModes != null) && (j < flashModes.size()); j++) {
						String flashMode = flashModes.get(j);
						if (!flashMode.equals(android.hardware.Camera.Parameters.FLASH_MODE_OFF)) {
							thisCameraInfo.flashSupported = true;
						}
					}

					long bestPixels = 0;
					List<android.hardware.Camera.Size> pictureSizes = p.getSupportedPictureSizes();
					for (int j = 0; (pictureSizes != null) && (j < pictureSizes.size()); j++) {
						android.hardware.Camera.Size size = pictureSizes.get(j);
						long pixels = (long) size.width * (long) size.height;
						if (bestPixels < pixels) {
							bestPixels = pixels;
							thisCameraInfo.picSupported = true;
							thisCameraInfo.maxPicX = size.width;
							thisCameraInfo.maxPicY = size.height;
						}
					}
					bestPixels = 0;
					List<android.hardware.Camera.Size> videoSizes = null;
					if (Build.VERSION.SDK_INT > 10) {
						videoSizes = getSupportedVideoSizes(p);
					}
					if (videoSizes == null) {
						videoSizes = p.getSupportedPreviewSizes();
					}
					for (int j = 0; (videoSizes != null) && (j < videoSizes.size()); j++) {
						android.hardware.Camera.Size size = videoSizes.get(j);
						long pixels = (long) size.width * (long) size.height;
						if (bestPixels < pixels) {
							bestPixels = pixels;
							thisCameraInfo.vidSupported = true;
							thisCameraInfo.maxVidX = size.width;
							thisCameraInfo.maxVidY = size.height;
						}
					}

					thisCameraInfo.flatCameraInfo = p.flatten();

					mCameras[i] = thisCameraInfo;
					c.release();
				} catch (Exception e) {
					mNumberOfCameras = 0;
				}
			}
		}
	}

	List<android.hardware.Camera.Size> getSupportedVideoSizes(Parameters p) {
		
		List<android.hardware.Camera.Size> videoSizes = p.getSupportedVideoSizes();
		return videoSizes;
	}

	int getSupportedFaces(Parameters p) {
		
		return p.getMaxNumDetectedFaces();
	}

	public String toString() {
		
		String result = "";
		for (int i = 0; (mCameras != null) && (i < mCameras.length); i++) {
			if (mCameras[i] != null) {
				result += (mCameras[i].isFrontFacing ? "front" : "back")
						+ " facing camera, flash: "
						+ (mCameras[i].flashSupported ? "supported" : "not supported")
						+ "\nface detection: "
						+ (mCameras[i].faceDetection ? "supported" : "not supported")
						+ (mCameras[i].picSupported ? ", picture capture is not supported" : (", picture resolution: " + mCameras[i].maxPicX
								+ " x " + mCameras[i].maxPicY))
						+ (mCameras[i].vidSupported ? ", video capture is not supported" : (", video resolution: " + mCameras[i].maxVidX
								+ " x " + mCameras[i].maxVidY)) + "\n";
			}
		}
		return result;
	}

	public String getRawData() {
		
		StringBuilder sb = new StringBuilder();
		sb.append("/** Camera **/").append("\n");
		sb.append("FEATURE_CAMERA: ").append(FEATURE_CAMERA).append("\n");
		sb.append("FEATURE_CAMERA_FRONT: ").append(FEATURE_CAMERA_FRONT).append("\n");
		for (int i = 0; (mCameras != null) && (i < mCameras.length) && (mCameras[i] != null); i++) {
			sb.append("CAMERA_FACING_FRONT: ").append(Boolean.toString(mCameras[i].isFrontFacing)).append("\n");
			sb.append("flat-info: ").append(mCameras[i].flatCameraInfo).append("\n");
		}

		sb.append("/** END **/").append("\n");

		return sb.toString();
	}
}
