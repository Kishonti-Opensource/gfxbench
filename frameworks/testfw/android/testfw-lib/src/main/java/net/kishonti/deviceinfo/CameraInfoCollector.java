/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

import java.util.ArrayList;
import java.util.List;

import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.util.Log;

@SuppressWarnings("deprecation")
public class CameraInfoCollector {

	private static final String TAG = "CameraInfoCollector";

	public List<CameraInfo> collect() {
		int n = Camera.getNumberOfCameras();
		List<CameraInfo> cis = new ArrayList<CameraInfo>();
		for (int cameraId = 0; cameraId < n; ++cameraId) {

			CameraInfo ci = new CameraInfo();
			android.hardware.Camera.CameraInfo cameraInfo = new android.hardware.Camera.CameraInfo();
			Camera.getCameraInfo(cameraId, cameraInfo);
			switch(cameraInfo.facing) {
			case android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK:
				ci.facing = "CAMERA_FACING_BACK";
				break;
			case android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT:
				ci.facing = "CAMERA_FACING_FRONT";
				break;
			default:
				ci.facing = Integer.toString(cameraInfo.facing);
			}
			try {
				collectParamters(cameraId, ci);
			} catch(RuntimeException e) {
				Log.e(TAG, "Failed to collect camera information: " + e.getLocalizedMessage());
			}
			cis.add(ci);
		}
		return cis;
	}

	private void collectParamters(int cameraId, CameraInfo ci) {
		Camera camera = Camera.open(cameraId);
		Parameters params = camera.getParameters();
		ci.max_exposure_compensation = params.getMaxExposureCompensation();
		ci.max_num_detected_faces = params.getMaxNumDetectedFaces();
		ci.max_num_focus_areas = params.getMaxNumFocusAreas();
		ci.max_num_metering_areas = params.getMaxNumMeteringAreas();
		ci.min_exposure_compensation = params.getMinExposureCompensation();
		ci.preferred_preview_size_for_video = params.getPreferredPreviewSizeForVideo();
		ci.supported_antibanding = params.getSupportedAntibanding();
		ci.supported_color_effects = params.getSupportedColorEffects();
		ci.supported_flash_modes = params.getSupportedFlashModes();
		ci.supported_focus_modes = params.getSupportedFocusModes();
		ci.supported_jpeg_thumbnail_sizes = params.getSupportedJpegThumbnailSizes();
		ci.supported_picture_sizes = params.getSupportedPictureSizes();
		ci.supported_preview_fps_range = params.getSupportedPreviewFpsRange();
		ci.supported_video_sizes = params.getSupportedVideoSizes();
		ci.supported_white_balance = params.getSupportedWhiteBalance();
		ci.supported_picture_formats = new ArrayList<String>();
		for (Integer format : params.getSupportedPictureFormats()) {
			ci.supported_picture_formats.add(ImageFormat.toString(format));
		}
		ci.supported_preview_formats = new ArrayList<String>();
		for (Integer format : params.getSupportedPreviewFormats()) {
			ci.supported_preview_formats.add(ImageFormat.toString(format));
		}
		camera.release();
	}
}
