/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;

import net.kishonti.swig.ApiDefinition;
import net.kishonti.swig.Descriptor;
import net.kishonti.swig.EGLGraphicsContext;
import net.kishonti.swig.GLFormat;
import net.kishonti.swig.TestBase;
import net.kishonti.swig.TestFactory;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;

public class TfwService extends Service {

	public static final String ACTION_START = "start";
	public static final String ACTION_STOP = "stop";
	private static final String TAG = "TfwService";
	private Descriptor mDescriptor;
	private TestBase mTest;
	private EGLGraphicsContext mGLContext;
	private String mBasepath;
	private boolean mForeground = true;

	@Override
	public void onCreate() {
		mBasepath = new File(Environment.getExternalStorageDirectory(), "/kishonti/tfw").getAbsolutePath();
		super.onCreate();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		try {
			if (ACTION_START.equals(intent.getAction())) {
				if (mTest != null) {
					throw new RuntimeException("Could not start. Service is already running");
				}
				Bundle b = intent.getExtras();
				if (b != null && b.containsKey("config")) {
					String config = b.getString("config");

					mDescriptor = new Descriptor();
					boolean success = mDescriptor.fromJsonString(config);
					if (!success) {
						throw new RuntimeException("Failed to parse descriptor");
					}
					prepare();
					if (mForeground) {
						startForeground(1, makeForegroundNotification());
					}
					new Thread("TfwService: " + mTest.name()) {
						@Override
						public void run() {
							TfwService.this.run();
						}
					}.start();

				} else {
					throw new RuntimeException("config key not set in Intent extra");
				}
			} else if (ACTION_STOP.equals(intent.getAction())) {
				if (mTest == null) {
					throw new RuntimeException("Could not stop. Service not running");
				} else {
					mTest.cancel();
					stopForeground(true);
					stopSelf(startId);
				}
			} else {
				Log.e(TAG, "Invalid action for service: " + intent.getAction());
			}
		} catch (Exception e) {
			Log.e(TAG, "Failed to handle intent: " + e.getLocalizedMessage());
			stopForeground(true);
			stopSelf(startId);
		}
		return Service.START_NOT_STICKY;
	}

	private Notification makeForegroundNotification() {
		Intent stop = new Intent(ACTION_STOP);
		stop.setClass(this, TfwService.class);
		Notification noti = new Notification.Builder(this)
        .setContentTitle("Service is running")
        .setSmallIcon(android.R.drawable.progress_indeterminate_horizontal)
        .setOngoing(false)
        .setAutoCancel(true)
        .setDeleteIntent(PendingIntent.getService(this, 0, stop, 0))
        .build();
		return noti;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		if (mTest != null && !mTest.isCancelled()) {
			mTest.cancel();
		}
		Log.i(TAG, "onDestroy");
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	private void prepare() {
		prepareDescriptor();
		createTest();
		prepareGraphics();

		String config = mDescriptor.toJsonString();
		mTest.setConfig(config);
		mTest.setName(mDescriptor.testId());
		mTest.setContext(this);
		if (mGLContext != null) {
			mTest.setGraphicsContext(mGLContext);
			mGLContext.detachThread();
		}
	}

	public void run() {
		if (mGLContext == null || !mGLContext.isValid()) {
			Log.e(TAG, "EGLGraphicsContext: not valid");
			return;
		}

		mGLContext.makeCurrent();

		VideoStreamFactory streamFactory = null;
		net.kishonti.swig.VideoStream videoStream = null;
		if (mDescriptor.rawConfigHasKey("video_stream")) {
			streamFactory = new VideoStreamFactory();
			Uri stream = Uri.parse(mDescriptor.rawConfigs("video_stream"));
			videoStream = streamFactory.open(this, stream);
		}

		mTest.setVideoStream(videoStream);
		boolean initok = mTest.init();
		if (initok) {
			Log.i(TAG, "running " + mTest.name());
			mTest.run();
		}
		mTest.setVideoStream(null);
		if (streamFactory != null) {
			streamFactory.close();
		}
		mTest.terminate();
		mTest = null;
		if (mGLContext != null) {
			mGLContext.detachThread();
		}
		mGLContext.destroy();
		mGLContext = null;
	}
	private void prepareDescriptor() {

		// update read/write paths
		net.kishonti.swig.Environment env = mDescriptor.env();

		File readPath = new File(mBasepath, "data/" + dataPrefix(mDescriptor));

		File writePath;
		if (env.writePath() != null && !env.writePath().isEmpty()) {
			writePath = new File(env.writePath());
		} else {
			writePath = new File(mBasepath, "data/" + dataPrefix(mDescriptor));
		}

		if (!(readPath.isDirectory() && readPath.exists())) {
			Log.e(TAG, "invalid read_path: " + readPath.getAbsolutePath());
		}
		if (!writePath.exists()) {
			Log.w(TAG, "invalid write_path: " + writePath.getAbsolutePath());
		}
		env.setReadPath(readPath.getAbsolutePath() + "/");
		env.setWritePath(writePath.getAbsolutePath() + "/");
	}

	private static String dataPrefix(Descriptor descriptor) {
		if (descriptor.dataPrefix() != null && !descriptor.dataPrefix().isEmpty()) {
			return descriptor.dataPrefix();
		} else {
			int end = descriptor.testId().indexOf('_');
			if (end < 0) {
				return descriptor.testId();
			} else {
				return descriptor.testId().substring(0, end);
			}
		}
	}
	private void prepareGraphics() {
		mDescriptor.env().setWidth(196);
		mDescriptor.env().setHeight(320);
		if (mGLContext == null) {
			ApiDefinition v = getPreferredESContextVersion();
			mGLContext = new EGLGraphicsContext();
			//mGLContext.setFormat(format);
			mGLContext.setFormat(new GLFormat(8, 8, 8, -1, -1, -1, -1, false));
			mGLContext.setContextVersion(v.major(), v.minor());
			if (!mGLContext.initPBufferSurface(mDescriptor.env().width(), mDescriptor.env().height())) {
				mGLContext.destroy();
			}
		}
		if (!mGLContext.isValid()) {
			throw new RuntimeException("Failed to init EGLGraphicsContext");
		}

	}

	private void createTest() {
		System.gc();
		if (mDescriptor.preloadLibs() != null) {
			for (int i = 0; i < mDescriptor.preloadLibs().size(); i++) {
				String preloadLib = mDescriptor.preloadLibs().get(i);
				try {
					System.loadLibrary(preloadLib);
				} catch(UnsatisfiedLinkError e) {
					Log.w(TAG, "Failed to preload lib: " + e.getLocalizedMessage());
				}
			}
		}
		TestFactory factory = TestFactory.test_factory(mDescriptor.factoryMethod());
		if (factory.valid()) {
			mTest = factory.create_test();
		} else {
			throw new RuntimeException("Failed to create C++ test factory for test_id: " + mDescriptor.testId() + ", factory_method: " + mDescriptor.factoryMethod());
		}
		if (mTest == null) {
			throw new RuntimeException("Failed to create test: " + mDescriptor.testId());
		}
	}

	private ApiDefinition getPreferredESContextVersion() {
			ApiDefinition ctxVersion = new ApiDefinition();
			ctxVersion.setMajor(Integer.MAX_VALUE); // not set
			ctxVersion.setMinor(Integer.MAX_VALUE); // not set

			// find the smallest version set in the descriptor
			if ((mDescriptor != null) && (mDescriptor.env() != null) && (mDescriptor.env().graphics() != null) && (mDescriptor.env().graphics().versions() != null)) {
				for (int i = 0; i < mDescriptor.env().graphics().versions().size(); i++) {
					ApiDefinition v = mDescriptor.env().graphics().versions().get(i);
					if (v.type() == ApiDefinition.Type.ES) {
						if (v.major() < ctxVersion.major()) ctxVersion = v;
					}
				}
			}
			if (ctxVersion.major() == Integer.MAX_VALUE) {
				ctxVersion.setMajor(2); // if not set, use 2 for default
				ctxVersion.setMinor(0);
			}

			return ctxVersion;
		}

}
