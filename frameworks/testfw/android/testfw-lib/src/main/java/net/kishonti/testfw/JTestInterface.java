/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

public interface JTestInterface {
    public void setContext(android.content.Context context);
    public String config();
    public void setConfig(String config);
    public boolean init();
    public net.kishonti.swig.GraphicsContext graphicsContext();
    public void setGraphicsContext(net.kishonti.swig.GraphicsContext ctx);
    public void setMessageQueue(net.kishonti.swig.MessageQueue msgQueue);
    public void setVideoStream(net.kishonti.swig.VideoStream stream);
    public void setVideoStream(int name, net.kishonti.swig.VideoStream stream);
    public void setRuntimeInfo(net.kishonti.swig.RuntimeInfo runtimeInfo);
    public void run();
    public String result();
    public void cancel();
    public boolean isCancelled();
    public void setArmed(boolean b);
    public boolean isArmed();
    public void setName(String name);
    public String name();
    public float progress();
    public boolean terminate();
    public String version();
}
