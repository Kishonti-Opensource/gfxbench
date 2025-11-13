/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import android.content.Context;
import net.kishonti.swig.GraphicsContext;
import net.kishonti.swig.MessageQueue;
import net.kishonti.swig.VideoStream;
import net.kishonti.swig.RuntimeInfo;

public abstract class JTestBase implements JTestInterface {
    private String mConfig;
    private boolean mArmed;
    private boolean mCanceled;
    private String mResult;
    private String mName;
    protected GraphicsContext mGraphicsContext;
    protected MessageQueue mMsgQueue;
    protected VideoStream mStream;
    protected RuntimeInfo mRuntimeInfo;
    protected Context mContext;

    @Override
    public void setContext(Context context) {
        mContext = context;
    }

    @Override
    public String config() {
        return mConfig;
    }

    @Override
    public void setConfig(String config) {
        mConfig = config;
    }

    @Override
    public boolean init() {
        return false;
    }

    @Override
    public void setGraphicsContext(GraphicsContext ctx) {
        mGraphicsContext = ctx;
    }

    @Override
    public GraphicsContext graphicsContext() {
        return mGraphicsContext;
    }
    @Override
    public void setMessageQueue(MessageQueue msgq) {
        mMsgQueue = msgq;
    }
    @Override
    public void setVideoStream(VideoStream stream) {
        mStream = stream;
    }

	@Override
	public void setVideoStream(int name, VideoStream stream) {
		// TODO: store streams in map, and support multiple streams
		if (name != 0) {
			throw new RuntimeException("stream name must be '0'");
		}
		mStream = stream;
	}

    @Override
    public void setRuntimeInfo(RuntimeInfo runtimeInfo) {
        mRuntimeInfo = runtimeInfo;
    }

    @Override
    public String result() {
        return mResult;
    }

    @Override
    public void cancel() {
        mCanceled = true;
    }

    @Override
    public boolean isCancelled() {
        return mCanceled;
    }

    @Override
    public void setArmed(boolean b) {
        mArmed = true;
    }

    @Override
    public boolean isArmed() {
        return mArmed;
    }

    @Override
    public void setName(String name) {
        mName = name;
    }

    @Override
    public String name() {
        return mName;
    }

    @Override
    public float progress() {
        return 0;
    }

    @Override
    public boolean terminate() {
        return true;
    }

    @Override
    public String version() {
        return "";
    }
}
