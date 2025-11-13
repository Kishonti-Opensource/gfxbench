/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;
import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.media.MediaPlayer;
import android.net.Uri;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;
import android.view.Surface;
import android.view.WindowManager;

public class VideoStreamFactory {

	public static class VideoStream extends net.kishonti.swig.VideoStream {
		private SurfaceTexture mSurfaceTexture;
		private net.kishonti.swig.FloatArray mCTexMatrix;
		private float[] mTexMatrix;

		public VideoStream() {
			super();
			int[] textures = new int[1];
			GLES20.glGenTextures(1, textures, 0);
			int texId = textures[0];
			GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texId);
			GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
			GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
			GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
			GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,	GLES20.GL_CLAMP_TO_EDGE);
			mSurfaceTexture = new SurfaceTexture(texId);
			mTexMatrix = new float[16];
			mCTexMatrix = new net.kishonti.swig.FloatArray(16);
		}

		public SurfaceTexture getSurfaceTexture() {
			return mSurfaceTexture;
		}

		public String decoderString() {
			return "Android MediaPlayer (HARDWARE VIDEO DECODE)";
		}

		public void updateTexImage() {
			mSurfaceTexture.getTransformMatrix(mTexMatrix);
			for (int i = 0; i < 16; ++i) {
				mCTexMatrix.setitem(i, mTexMatrix[i]);
			}
			setTransformMatrix(mCTexMatrix.cast());
			mSurfaceTexture.updateTexImage();
		}

	}

	private static final String TAG = "VideoStreamFactory";

	private Camera mCamera;
	private int mCameraId;
	private MediaPlayer mPlayer;
	private VideoStream mVideoStream;
	private int mStreamWidth;
	private int mStreamHeight;
	private File mBasepath = null;
	private VideoFrameExtractor mVideoFrameExtractor;


	public net.kishonti.swig.VideoStream open(Context context, Uri stream) {
		try {
			if (stream.getScheme().equals("camera")) {
				return openCamera(context, stream);
			} if (stream.getScheme().equals("frames")) {
				return openVideoStreamExtractor(stream);
			} else {
				return openMedia(context, stream);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

	public void setBasepath(String basepath) {
		mBasepath = new File(basepath);
	}

	private net.kishonti.swig.VideoStream openVideoStreamExtractor(Uri stream) {
		try {
			Uri uri = makeUriPathAbsolute(stream);
			mVideoFrameExtractor = new VideoFrameExtractor(uri.getPath());
			mVideoFrameExtractor.setEnableRestart(true);
			return mVideoFrameExtractor.getVideoStream();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
	}

	private VideoStream openMedia(Context context, Uri stream) {
		mPlayer = MediaPlayer.create(context, makeUriPathAbsolute(stream));
		mVideoStream = new VideoStream();
		mVideoStream.setName(stream.toString());
		mVideoStream.setSize(mPlayer.getVideoWidth(), mPlayer.getVideoHeight());
		mPlayer.setSurface(new Surface(mVideoStream.getSurfaceTexture()));
		mPlayer.start();
		return mVideoStream;
	}

	private Uri makeUriPathAbsolute(Uri stream) {
		if (mBasepath != null) {
			Uri.Builder b = stream.buildUpon();
			File file = new File(mBasepath, stream.getPath());
			b.path(file.getAbsolutePath());
			b.build();
			return b.build();
		}
		return stream;
	}

	private VideoStream openCamera(Context context, Uri stream) {
		String host = stream.getHost();
		mCameraId = Integer.valueOf(host);
		mCamera = Camera.open(mCameraId);
		Camera.Parameters params = mCamera.getParameters();
		List<Size> previewSizes = params.getSupportedPreviewSizes();
		int i = 0;
		long maxNumPixels = 0;
		for (Size size : previewSizes) {
			Log.i(TAG, String.format("Preview size #%d: %dx%d", i++, size.width, size.height));
			long numPixels = size.width * size.height;
			if (numPixels > maxNumPixels) {
				mStreamWidth = size.width;
				mStreamHeight = size.height;
				maxNumPixels = numPixels;
			}
		}

		String queryWidth = stream.getQueryParameter("w");
		String queryHeight = stream.getQueryParameter("h");
		if (queryWidth != null && queryHeight != null) {
			try {
				int w = Integer.valueOf(queryWidth);
				int h = Integer.valueOf(queryHeight);
				mStreamWidth = w;
				mStreamHeight = h;
			} catch(NumberFormatException e) {
				Log.e(TAG, "Failed to read preview dimensions: " + e.getLocalizedMessage());
			}
		}

		List<int[]> fpsRanges = params.getSupportedPreviewFpsRange();
		int previewFpsRangeMin = 0;
		int previewFpsRangeMax = 0;
		for (int[] fpsRange : fpsRanges) {
			int minFps = fpsRange[Camera.Parameters.PREVIEW_FPS_MIN_INDEX];
			int maxFps = fpsRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX];
			Log.i(TAG, String.format("Fps range (1000x): %d -- %d", minFps, maxFps));
			if (minFps >= previewFpsRangeMin && maxFps >= previewFpsRangeMax) {
				previewFpsRangeMin = minFps;
				previewFpsRangeMax = maxFps;
			}
		}

		String queryPrefiewFpsMin = stream.getQueryParameter("fpsmin");
		String queryPrefiewFpsMax = stream.getQueryParameter("fpsmax");
		if (queryPrefiewFpsMin != null && queryPrefiewFpsMax != null) {
			try {
				int min = Integer.valueOf(queryPrefiewFpsMin);
				int max = Integer.valueOf(queryPrefiewFpsMax);
				previewFpsRangeMin = min;
				previewFpsRangeMax = max;
			} catch(NumberFormatException e) {
				Log.e(TAG, "Failed to read preview dimensions: " + e.getLocalizedMessage());
			}
		}

		params.setPreviewFpsRange(previewFpsRangeMin, previewFpsRangeMax);
		params.setPreviewSize(mStreamWidth, mStreamHeight);
		List<String> supportedFocusModes = mCamera.getParameters().getSupportedFocusModes();
		boolean hasAutoFocus = supportedFocusModes != null && supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO);
		if (hasAutoFocus) {
			params.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
		}
		mCamera.setParameters(params);

		// update mStreamWidth/Height
		setCameraDisplayOrientation(context, mCameraId);

		mVideoStream = new VideoStream();
		mVideoStream.setName(stream.toString());
		mVideoStream.setSize(mStreamWidth, mStreamHeight);
		try {
			mCamera.setPreviewTexture(mVideoStream.getSurfaceTexture());
			mCamera.startPreview();
		} catch (IOException e1) {
			mVideoStream = null;
			e1.printStackTrace();
		}
		return mVideoStream;
	}

	private void setCameraDisplayOrientation(Context context, int cameraId) {
		android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
		android.hardware.Camera.getCameraInfo(cameraId, info);
		WindowManager wmgr = (WindowManager) context
				.getSystemService(Context.WINDOW_SERVICE);
		int rotation = wmgr.getDefaultDisplay().getRotation();
		int degrees = 0;
		switch (rotation) {
		case Surface.ROTATION_0:
			degrees = 0;
			break;
		case Surface.ROTATION_90:
			degrees = 90;
			break;
		case Surface.ROTATION_180:
			degrees = 180;
			break;
		case Surface.ROTATION_270:
			degrees = 270;
			break;
		}

		int result;
		if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
			result = (info.orientation + degrees) % 360;
			result = (360 - result) % 360; // compensate the mirror
		} else { // back-facing
			result = (info.orientation - degrees + 360) % 360;
		}
		mCamera.setDisplayOrientation(result);
		if (result == 90 || result == 270) {
			int tmp = mStreamWidth;
			mStreamWidth = mStreamHeight;
			mStreamHeight = tmp;
		}
	}

	public void close() {
		if (mCamera != null) {
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
		if (mPlayer != null) {
			mPlayer.stop();
			mPlayer.release();
			mPlayer = null;
		}
		if (mVideoFrameExtractor != null) {
			mVideoFrameExtractor.close();
			mVideoFrameExtractor = null;
		}
		if (mVideoStream != null) {
			mVideoStream.delete();
			mVideoStream = null;
		}
	}
}
