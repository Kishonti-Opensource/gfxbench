/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import android.content.ComponentName;
import android.content.pm.*;
import android.content.Context;
import org.json.*;

import org.apache.commons.io.FileUtils;

import net.kishonti.NativeSignalReceiver;
import net.kishonti.swig.CompositeDescriptor;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.TfwMessageQueue;
import net.kishonti.swig.Descriptor;

import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowMetrics;
import android.widget.LinearLayout;

public class TfwActivity extends Activity implements
        NativeSignalReceiver.Handler, SurfaceHolder.Callback {

    private static final String TAG = "TfwActivity";
    protected List<TestRunner> mRunners;
    private int mPreparedRunnerCount;
    private int mFinishedRunnerCount;
    private Handler mHandler;
    private boolean mMovedToBackground = false;
    private Runnable mPoller;

    public static final String ACTION_TFW_RESULT = "net.kishonti.testfw.ACTION_TFW_RESULT";
    private static final String KEY_EXTRA_CONFIG = "config";
    private static final String KEY_EXTRA_TEST_ID = "test_id";
    private static final String KEY_EXTRA_FACTORY_METHOD = "factory_method";
    private static final String KEY_EXTRA_JCLASS = "jclass";
    private static final String KEY_EXTRA_DESCRIPTOR_FILE = "descriptor";
    private static final String KEY_EXTRA_PRELOAD_LIBS = "preload_libs";
    private static final String KEY_EXTRA_BASE_PATH = "base_path";
    public static final int RESULT_FAILED = RESULT_FIRST_USER + 1;
    public static final int RESULT_UNKNOWN_MSG = RESULT_FIRST_USER + 2;

    private String mCategory;
    private boolean mPaused = true; // paused before onResume, and after onPause
    private CountDownLatch mRunSync;
    private String mTestId = "unknown";
    private int mWidth;
    private int mHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        mCategory = getPackageName();
        mRunners = new ArrayList<>();
        mPreparedRunnerCount = 0;
        mFinishedRunnerCount = 0;

        NativeSignalReceiver.theHandler = this;

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        List<String> preloadLibs = new ArrayList<String>();
        String basepath = null;
        Intent intent = getIntent();
        Bundle b = intent.getExtras();
        if (b != null) {
            basepath = b.getString(KEY_EXTRA_BASE_PATH);
            String extra_libs = b.getString(KEY_EXTRA_PRELOAD_LIBS);
            if (extra_libs != null) {
                preloadLibs = Arrays.asList(extra_libs.split(";"));
            }

            if (b.containsKey("brightness")) {
                WindowManager.LayoutParams lp = getWindow().getAttributes();
                lp.screenBrightness = b.getFloat("brightness", 1.0f);
                getWindow().setAttributes(lp);
            }
        }

        try {
            List<Descriptor> descriptors = getDescriptors(b);
            mHandler = new TfwHandler(this);
            List<SurfaceView> surfaceViews = initContentView(descriptors);

            if (surfaceViews.size() != descriptors.size()) {
                throw new RuntimeException(
                        "Number of SurfaceViews and Descriptors does not match");
            }
            mRunSync = new CountDownLatch(descriptors.size());
            for (int i = 0; i < descriptors.size(); ++i) {
                Descriptor descriptor = descriptors.get(i);

                // TODO HACK for library preloading
                for (String preloadLib : preloadLibs) {
//					NativeLibraryLoader.load(this, preloadLib);

                    if (preloadLib.contains("__TO__")) {
                        String[] swap = preloadLib.split("__TO__");
                        for (int j = 0; j < descriptor.preloadLibs().size(); ++j) {
                            if (descriptor.preloadLibs().get(j).equals(swap[0])) {
                                descriptor.preloadLibs().set(j, swap[1]);
                            }
                        }

                    } else {
                        descriptor.preloadLibs().add(preloadLib);
                    }
                }
                TestRunner runner = new TestRunner(this, descriptor, mHandler, mRunSync);
                SurfaceView v = surfaceViews.get(i);
                v.setKeepScreenOn(true);
                v.getHolder().addCallback(this);
                if (descriptor.env().width() > 0 && descriptor.env().height() > 0) {
                    v.getHolder().setFixedSize(descriptor.env().width(), descriptor.env().height());
                }
                runner.setSurfaceView(v);

                int androidApiLevel = Build.VERSION.SDK_INT;
                if (androidApiLevel >= 30) {
                    Display display = getWindowManager().getDefaultDisplay();
                    Display.Mode[] displayModes = display.getSupportedModes();
                    WindowMetrics windowMetrics = getWindowManager().getCurrentWindowMetrics();
                    log(String.format("Current display bounds: %d x %d", windowMetrics.getBounds().width(), windowMetrics.getBounds().height()));
                    int totalPixels = windowMetrics.getBounds().width() * windowMetrics.getBounds().height();
                    float fps = 0.0f;
                    Display.Mode selectedMode = null;

                    // Select highest frame rate for current resolution
                    log("display.getSupportedModes():");
                    for (Display.Mode displayMode : displayModes) {
                        float displayFps = displayMode.getRefreshRate();
                        int width = displayMode.getPhysicalWidth();
                        int height = displayMode.getPhysicalHeight();
                        log(String.format("    %d x %d  %f fps", width, height, displayFps));
                        int numPixels = width * height;
                        if (numPixels == totalPixels && displayFps > fps) {
                            fps = displayFps;
                            selectedMode = displayMode;
                        }
                    }

                    if (selectedMode == null)
                    {
                        fps = -1.0f;
                        log("No appropriate display mode. Not setting refresh rate");
                    }

                    runner.setSurfaceFrameRate(fps);
                }

                if (basepath != null) {
                    runner.setBasepath(basepath);
                }
                mRunners.add(runner);
            }
        } catch (Exception e) {
            finishWithFatalError(mTestId, e.toString());
        }
    }

    private List<Descriptor> getDescriptors(Bundle b) throws IOException {
        String config = b.getString(KEY_EXTRA_CONFIG);
        String descriptorFile = b.getString(KEY_EXTRA_DESCRIPTOR_FILE);
        String testId = b.getString(KEY_EXTRA_TEST_ID);
        String factoryMethod = b.getString(KEY_EXTRA_FACTORY_METHOD);
        String jclass = b.getString(KEY_EXTRA_JCLASS);

        List<Descriptor> descriptors = new ArrayList<Descriptor>();
        if (config != null) {
            try {
                JSONObject obj = new JSONObject(config);
            } catch (Exception e) {
                config = FileUtils.readFileToString(new File(config));
            }
        }
        // first check config
        if (config == null && descriptorFile != null) {
            config = FileUtils.readFileToString(new File(descriptorFile));
        }
        if (config != null) {
            CompositeDescriptor compositeDescriptor = new CompositeDescriptor();
            if (compositeDescriptor.fromJsonString(config)) {
                mTestId = compositeDescriptor.testId();
                for (int i = 0; i < compositeDescriptor.size(); ++i) {
                    Descriptor d = compositeDescriptor.descriptor(i);
                    descriptors.add(d);
                }
            } else {
                throw new RuntimeException("Failed to parse descriptor");
            }
        } else if (testId != null) {
            mTestId = testId;
            Descriptor descriptor = new Descriptor();
            descriptor.setTestId(testId);
            if (factoryMethod == null) {
                descriptor.setFactoryMethod(testId);
            } else {
                descriptor.setFactoryMethod(factoryMethod);
            }
            try {
                descriptor.setJclass(jclass);
            } catch (Exception e) {
            }
            descriptors.add(descriptor);
        }
        return descriptors;
    }

    protected List<SurfaceView> initContentView(List<Descriptor> descriptors) {
        int surfaceCnt = descriptors.size();
        List<SurfaceView> surfaceViews = new ArrayList<SurfaceView>();
        if (surfaceCnt == 1) {
            SurfaceView v = new SurfaceView(this);
            surfaceViews.add(v);
            setContentView(v);
        } else if (1 < surfaceCnt && surfaceCnt < 4) {
            LinearLayout layout = new LinearLayout(this);
            layout.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                    LayoutParams.MATCH_PARENT));

            for (int i = 0; i < surfaceCnt; ++i) {
                SurfaceView v = new SurfaceView(this);
                surfaceViews.add(v);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
                params.weight = 1.0f;
                v.setLayoutParams(params);
                layout.addView(v);
            }
            setContentView(layout);
        } else if (surfaceCnt == 4) {
            GridLayout grid = new GridLayout(this);
            grid.setColumnCount(2);
            grid.setRowCount(2);
            for (int i = 0; i < surfaceCnt; ++i) {
                SurfaceView v = new SurfaceView(this);
                surfaceViews.add(v);
                grid.addView(v);
            }
            setContentView(grid);
        }
        return surfaceViews;
    }

    private void startRunners() {
        for (TestRunner runner : mRunners) {
            runner.start();
        }
        mPoller = new Runnable() {
            @Override
            public void run() {
                for (TestRunner runner : mRunners) {
                    onProgressChanged(runner, runner.getTest().progress());
                }
                mHandler.postDelayed(this, 100);
            }
        };
        mHandler.post(mPoller);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        log("onFocus changed " + this + " hasFocus: " + hasFocus);
        if (!hasFocus) {
            cancelRunners();
        }
    }

    public TfwMessageQueue getMessageQueue(int i) {
        TestRunner runner = mRunners.get(i);
        if (runner != null) {
            return runner.getMessageQueue();
        } else {
            return null;
        }
    }

    private static void sendImplicitBroadcast(Context ctxt, Intent i) {
      PackageManager pm=ctxt.getPackageManager();
      List<ResolveInfo> matches=pm.queryBroadcastReceivers(i, 0);

      for (ResolveInfo resolveInfo : matches) {
        Intent explicit=new Intent(i);
        ComponentName cn=
          new ComponentName(resolveInfo.activityInfo.applicationInfo.packageName,
            resolveInfo.activityInfo.name);

        explicit.setComponent(cn);
        ctxt.sendBroadcast(explicit);
      }
    }

    private void finishWithResult() {
        mHandler.removeCallbacks(mPoller);
        Intent data = new Intent(ACTION_TFW_RESULT);
        data.addCategory(mCategory);

        int resultCode = RESULT_FAILED;
        ResultGroup rg = null;
        if (mRunners.size() == 1) {
            TestRunner t = mRunners.get(0);
            rg = t.getResults();
            resultCode = t.getStatus();
        } else {
            // TODO: this is a temporary solution for creating a composite result
			rg = new ResultGroup();
			ResultGroup mRg = new ResultGroup();
            for (TestRunner t : mRunners) {
                ResultGroup g0 = t.getResults();
				mRg.merge(g0);
            }

			Result r = new Result();
			Double computedResult = new Double(1.0);

			for (int i = 0; i < mRg.results().size(); ++i){
				computedResult *= mRg.results().get(i).score();
			}

			computedResult = (Double) Math.pow(computedResult, (double)((double)1/mRg.results().size()));
			r.setTestId(mTestId);
			r.setScore(computedResult);
			r.setStatus(Result.Status.OK);
			rg.addResult(r);
        }

        // set test_id here for composite tests
        rg.setTestId(mTestId);

        String result = rg.toJsonString();
		for (TestRunner t : mRunners) {
            t.releaseGraphics();
            t.releaseTest();
        }

        // if activity was moved to background with HOME or TURN_OFF, we should
        // close the session
        data.putExtra("close_session", mMovedToBackground);
        data.putExtra("result", result);
        setResult(resultCode, data);
        sendImplicitBroadcast(this, data);
        finish();
        mPoller = null;
        System.gc();
    }

    private void finishWithFatalError(String testId, String message) {
        Intent intent = new Intent(ACTION_TFW_RESULT);
        intent.addCategory(mCategory);

        Result res = TestUtils.createFailedResult(testId, testId, message);
        ResultGroup r = TestUtils.createSingleResultList(res);
        log("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX2");

        String s = r.toJsonString();
        intent.putExtra("result", s);
        setResult(RESULT_FAILED, intent);
        sendImplicitBroadcast(this, intent);
        finish();
    }

    protected void onTestInitialized(TestRunner t) {
        log(t.getTestId() + ": initialized");
        mHandler.removeCallbacks(mPoller);
        mPoller = null;
        mRunSync.countDown();
    }

    protected void onTestDone(TestRunner t) {
        ++mFinishedRunnerCount;
        if (mFinishedRunnerCount == mRunners.size()) {
            finishWithResult();
        }
    }

    public void onProgressChanged(TestRunner runner, float progress) {
    }

    @Override
    public void nativeSignalReceived(int signal) {
        System.err.println("Oooops... we gonna die");
        //startActivity(new Intent(this, CrashHandler.class));
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int frmt, int width,
                               int height) {
        log("Surface changed: " + holder.toString());

        Boolean wasRunning = false;
        for (TestRunner runner : mRunners) {
            if (runner.isAlive()) {
                wasRunning = true;
            } else if (runner.getHolder() == holder) {
                runner.setEnvironmentSize(width, height);
                if (holder.isCreating()) {
                    runner.setup();
                    ++mPreparedRunnerCount;
                }
            }
        }
        boolean changedResolution = (mWidth != holder.getSurfaceFrame().width()) ||  (mHeight != holder.getSurfaceFrame().height());
        if (wasRunning && changedResolution) {
            log("ResolutionChange cancel running test");
            cancelRunners();
        } else if (!mPaused && mPreparedRunnerCount == mRunners.size()) {
            log("startRunners");
            startRunners();
        }
    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        log("Surface created: " + holder.toString());
        mWidth = holder.getSurfaceFrame().width();
        mHeight = holder.getSurfaceFrame().height();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        log("surfaceDestroyed: " + holder.toString());
        // TODO: mGLCtx.destroy();
    }

    @Override
    public void onResume() {
        log("onResume");
        super.onResume();
        mPaused = false;
    }

    @Override
    public void onPause() {
        log("onPause");
        mPaused = true;
        mMovedToBackground = !isFinishing();
        cancelRunners();
        super.onPause();
    }

    @Override
    public void onBackPressed() {
        cancelRunners();
    }

    private void cancelRunners() {
        for (TestRunner runner : mRunners) {
            runner.cancel();
        }
    }

    private static class TfwHandler extends Handler {
        private final WeakReference<TfwActivity> mTarget;

        TfwHandler(TfwActivity target) {
            mTarget = new WeakReference<TfwActivity>(target);
        }

        @Override
        public void handleMessage(Message msg) {
            TfwActivity a = mTarget.get();
            if (a != null) {
                TestRunner t = (TestRunner) msg.obj;
                a.log("message received: " + msg.what);
                switch (msg.what) {
                    case TestRunner.MSG_INITIALIZED:
                        a.onTestInitialized(t);
                        break;
                    case TestRunner.MSG_DONE:
                        a.onTestDone(t);
                        break;
                    default:
                        a.log("Unknown msg: " + msg.toString());
                }
            }
        }
    }

    public ResultGroup getResult(int i) {
        return mRunners.get(i).getResults();
    }

    private void log(String message) {
        Log.i(TAG, "[" + this + "]: " + message);
    }
}