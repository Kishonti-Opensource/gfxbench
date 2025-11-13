/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import android.annotation.SuppressLint;
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Surface;
import android.view.View;
import android.view.View.OnTouchListener;
import net.kishonti.swig.ApiDefinition;
import net.kishonti.swig.Config;
import net.kishonti.swig.GraphicsContext;
import net.kishonti.swig.EGLGraphicsContext;
import net.kishonti.swig.AndroidVulkanGraphicsContext;
import net.kishonti.swig.OVRGraphicsContext;
import net.kishonti.swig.GLFormat;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.RuntimeInfo;
import net.kishonti.swig.StringVector;
import net.kishonti.swig.TestFactory;
import net.kishonti.swig.TfwMessageQueue;
import net.kishonti.swig.Descriptor;
import net.kishonti.swig.testfw;
import net.kishonti.swig.testfwJNI;


public class TestRunner extends Thread {
	private JTestInterface mTest;
	private SurfaceView mSurfaceView;
	private final Descriptor mDescriptor;
	private GraphicsContext mGLContext;
	private final TfwMessageQueue mMsgQueue;
	private String mBasepath;
	private long mMinimumInitMillisecs;

	private final Handler mHandler;
	private boolean mCancelled;
	private Context mContext;
	private RuntimeInfo mRuntimeInfo;
	private CountDownLatch mRunSync;
	private int mStatus = TfwActivity.RESULT_FAILED;
	private static final String TAG = "Runner";
	public static final int MSG_INITIALIZED = 1;
	public static final int MSG_DONE = 2;

	private float mSurfaceFrameRate = -1.0f;

	public static Class<? extends JTestFactory> sJTestFactory = JTestFactory.class;

	public TestRunner(Context context, Descriptor descriptor, Handler handler, CountDownLatch runSync) throws IllegalArgumentException, InstantiationException, IllegalAccessException, InvocationTargetException, NoSuchMethodException {
		super("Runner: " + descriptor.testId());
		mContext = context;
		mHandler = handler;
		mRunSync = runSync;
		mBasepath = new File(Environment.getExternalStorageDirectory(), "/kishonti/tfw").getAbsolutePath();
		mDescriptor = descriptor;
		mMsgQueue = new TfwMessageQueue();
		mRuntimeInfo = new AndroidRuntimeInfo(context);

		mMinimumInitMillisecs = 2000;
		mCancelled = false;
		createTest();
	}

	public void setSurfaceView(SurfaceView surfaceView) {
		mSurfaceView = surfaceView;
		mSurfaceView.setOnTouchListener(new OnTouchListener() {

			@SuppressLint("ClickableViewAccessibility")
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				net.kishonti.swig.Message msg = new net.kishonti.swig.Message(
						event.getAction(), Math.round(event.getX()), Math.round(event.getY()), event.getFlags());
				mMsgQueue.push_back(msg);
				return true;
			}
		});
	}

	public void setSurfaceFrameRate(float fps) {
		mSurfaceFrameRate = fps;
	}

	public void setBasepath(String basepath) {
		mBasepath = basepath;
	}

	public void setMinimumInitMillisecs(long minimumInitMillisecs) {
		mMinimumInitMillisecs = minimumInitMillisecs;
	}

	public void setup() {
		prepareDescriptor();
		prepareGraphics();

		String config = mDescriptor.toJsonString();

		mTest.setConfig(config);
		mTest.setName(mDescriptor.testId());
		mTest.setMessageQueue(mMsgQueue);
		mTest.setContext(mContext);
		mTest.setRuntimeInfo(mRuntimeInfo);
		if (mGLContext != null) {
			mTest.setGraphicsContext(mGLContext);
			mGLContext.detachThread();
		}
	}

	private void prepareGraphics() {
		if (mSurfaceView == null) {
			return;
		}

		Surface surface = mSurfaceView.getHolder().getSurface();
		int androidApiLevel = Build.VERSION.SDK_INT;
		if (androidApiLevel >= 30 && mSurfaceFrameRate > 0.0f) {
			Log.i(TAG, String.format("surface.setFrameRate(%f)", mSurfaceFrameRate));
			surface.setFrameRate(mSurfaceFrameRate, Surface.FRAME_RATE_COMPATIBILITY_DEFAULT);
		}

		GLFormat format;
		try {
			Config cfg = mDescriptor.env().graphics().config();
			if (cfg.samples() != -1 && Build.FINGERPRINT.contains("generic")) {
				Log.w(TAG, "Emulator detected; disabling multisamples");
				cfg.setSamples(-1);
			}
			format = new GLFormat(cfg.red(), cfg.green(),
					cfg.blue(), cfg.alpha(), cfg.depth(),
					cfg.stencil(), cfg.samples(), cfg.isExact());
		} catch (Exception e) {
			Log.e(TAG, e.getLocalizedMessage());
			format = new GLFormat();
		}
		if (mGLContext == null) {
			ApiDefinition v = getPreferredESContextVersion();
			if(mDescriptor.rawConfigb("ovr", false)) {
				mGLContext = new OVRGraphicsContext();

				if (Build.FINGERPRINT.contains("generic")) {
					((OVRGraphicsContext)mGLContext).setUseDefaultChooseConfig(true);
				}
				((OVRGraphicsContext)mGLContext).setFormat(format);
				((OVRGraphicsContext)mGLContext).setContextVersion(v.major(), v.minor());

				if (!((OVRGraphicsContext)mGLContext).initWindowSurface(surface, mContext)) {
					((OVRGraphicsContext)mGLContext).destroy();
				}

			} else {
				if (v.type() == ApiDefinition.Type.ES) {

					mGLContext = new EGLGraphicsContext();	

					if (Build.FINGERPRINT.contains("generic")) {
						((EGLGraphicsContext)mGLContext).setUseDefaultChooseConfig(true);
					}
					((EGLGraphicsContext)mGLContext).setFormat(format);
					((EGLGraphicsContext)mGLContext).setContextVersion(v.major(), v.minor());

					if (!((EGLGraphicsContext)mGLContext).initWindowSurface(surface)) {
						((EGLGraphicsContext)mGLContext).destroy();
					}
				} else if (v.type() == ApiDefinition.Type.VULKAN) {
					mGLContext = new AndroidVulkanGraphicsContext();
					if (!((AndroidVulkanGraphicsContext)mGLContext).initWindowSurface(surface)) {
						((AndroidVulkanGraphicsContext)mGLContext).destroy();
					}
				}
			}
		}
		if (!mGLContext.isValid()) {
			throw new RuntimeException("Failed to init GraphicsContext");
		}

	}

	@Override
	public void run() {
		Message msg = null;
		long startedAt = System.currentTimeMillis();

		GraphicsContext ctx = mTest.graphicsContext();
		if (ctx != null) {
			ctx.makeCurrent();
		}

		List<VideoStreamFactory> streamFactories = new ArrayList<VideoStreamFactory>();
		List<String> streamUris = new ArrayList<String>();
		if (mDescriptor.rawConfigHasKey("video_streams")) {
			StringVector sv = mDescriptor.rawConfigsv("video_streams");
			for (int i = 0; i < sv.capacity(); ++i) {
				streamUris.add(sv.get(i));
			}
		} else if (mDescriptor.rawConfigHasKey("video_stream")) {
			streamUris.add(mDescriptor.rawConfigs("video_stream"));
		}
		int streamName = 0;
		for (String streamUri : streamUris) {
			VideoStreamFactory streamFactory = new VideoStreamFactory();
			streamFactory.setBasepath(mBasepath);
			Uri stream = Uri.parse(streamUri);
			net.kishonti.swig.VideoStream videoStream = streamFactory.open(mContext, stream);
			if (videoStream != null) {
				videoStream.setName(streamUri);
				mTest.setVideoStream(streamName++, videoStream);
				streamFactories.add(streamFactory);
			} else {
				Log.e(TAG, "Failed to open stream: " + streamUri);
			}
		}

		boolean initok = mTest.init();
		msg = Message.obtain(mHandler, MSG_INITIALIZED, 0, 0, this);
		msg.sendToTarget();

		int status = TfwActivity.RESULT_FAILED;
		try {
			mRunSync.await();
		} catch (InterruptedException e1) {
		}

		if (initok && !mTest.isCancelled()) {
			Log.i(TAG, "loaded: " + mTest.name());

			// Check init time
			long elapsed = System.currentTimeMillis() - startedAt;
			if (elapsed < mMinimumInitMillisecs) {
				// Let the loading screen stay a little longer
				try {
					sleep(mMinimumInitMillisecs - elapsed);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			Log.i(TAG, "running " + mTest.name());
			mTest.run();
			if (!mTest.isCancelled()) {
				mStatus = TfwActivity.RESULT_OK;
				Log.i(TAG, "finished: " + mTest.name());
			}
		}
		mTest.terminate();
		if (mTest.isCancelled()) {
			Log.i(TAG, "cancelled: " + mTest.name());
			mStatus = TfwActivity.RESULT_CANCELED;
			mCancelled = true;
		} else if (!initok) {
			Log.e(TAG, "Failed to init test: " + mTest.name());
			mStatus = TfwActivity.RESULT_FAILED;
		}
		streamName = 0;
		for(VideoStreamFactory streamFactory : streamFactories) {
			mTest.setVideoStream(streamName++, null);
			streamFactory.close();
		}
		streamFactories.clear();

		if (ctx != null) {
			ctx.detachThread();
		}

		msg = Message.obtain(mHandler, MSG_DONE, status, 0, this);
		msg.sendToTarget();
	}

	public void cancel() {
		if (mCancelled == false && mTest != null) {
			mTest.cancel();
		}
		mCancelled = true;
	}

	public JTestInterface getTest() {
		return mTest;
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

		if (BuildConfig.DEBUG) {
			Log.i(TAG, "read_path: " + readPath.getAbsolutePath());
			Log.i(TAG, "write_path: " + writePath.getAbsolutePath());
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

	public void setEnvironmentSize(int width, int height) {
		mDescriptor.env().setWidth(width);
		mDescriptor.env().setHeight(height);
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

	private void createTest() throws IllegalArgumentException, InstantiationException, IllegalAccessException, InvocationTargetException, NoSuchMethodException {
		System.gc();
		if (mDescriptor.jclass() != null && !mDescriptor.jclass().isEmpty()) {
			JTestFactory factory = sJTestFactory.getConstructor(String.class).newInstance(mDescriptor.jclass());
			mTest = factory.create_test();
		} else {
			if (mDescriptor.preloadLibs() != null) {
				for (int i = 0; i < mDescriptor.preloadLibs().size(); i++) {
					String preloadLib = mDescriptor.preloadLibs().get(i);
					try {
						NativeLibraryLoader.load(mContext, preloadLib);
					} catch(UnsatisfiedLinkError e) {
						Log.w(TAG, "Failed to preload lib: " + e.getLocalizedMessage());
					} catch (IOException e) {
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
		if ((mDescriptor != null) && (mDescriptor.env() != null) &&
			(mDescriptor.env().graphics() != null) &&
			(mDescriptor.env().graphics().versions() != null)) {

			for (int i = 0; i < mDescriptor.env().graphics().versions().size(); i++) {

				ApiDefinition v = mDescriptor.env().graphics().versions().get(i);
				if (v.type() == ApiDefinition.Type.ES || v.type() == ApiDefinition.Type.VULKAN) {
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

	public String getTestId() {
		return mDescriptor.testId();
	}

	public int getStatus() {
		return mStatus;
	}

	public TfwMessageQueue getMessageQueue() {
		return mMsgQueue;
	}

	public ResultGroup getResults() {
		ResultGroup rg = new ResultGroup();
		String s = mTest.result();
		rg.fromJsonString(s);
		return rg;
	}

	public SurfaceHolder getHolder() {
		return mSurfaceView.getHolder();
	}

	public void releaseGraphics() {
		if (mGLContext != null && mGLContext.isValid()) {

			if(mGLContext instanceof EGLGraphicsContext) {

				((EGLGraphicsContext)mGLContext).destroy();
			} else if(mGLContext instanceof AndroidVulkanGraphicsContext) {

				((AndroidVulkanGraphicsContext)mGLContext).destroy();
			} else if(mGLContext instanceof OVRGraphicsContext) {

				((OVRGraphicsContext)mGLContext).destroy();
			}
		}
	}

	public void releaseTest() {
		mTest = null;
	}
}
