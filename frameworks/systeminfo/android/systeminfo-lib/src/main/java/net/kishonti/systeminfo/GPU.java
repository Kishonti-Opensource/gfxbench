/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.util.ArrayList;
import java.util.HashMap;

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;

import static javax.microedition.khronos.egl.EGL10.*;

public class GPU implements Runnable {

	class GLInfo {
		public String version;
		public String renderer;
		public String vendor;
	}

	public enum OPENGL_TYPE {
		OpenGL_ES_2, OpenGL_ES_3
	}

	private ArrayList<HashMap<String, String>> mData = new ArrayList<HashMap<String, String>>();

	@Override
	public void run() {

		try {
			boolean es3Support = false;
			GLInfo infoV2 = getGLInformation(OPENGL_TYPE.OpenGL_ES_2);
			if (infoV2.renderer.contains("Vivante")) {

				es3Support = hasES3(infoV2.version);
			} else {
				if (android.os.Build.VERSION.SDK_INT > 17) {

					GLInfo infoV3 = getGLInformation(OPENGL_TYPE.OpenGL_ES_3);
					if (infoV3 != null) {

						es3Support = hasES3(infoV3.version);
					}
				}
			}

			if (es3Support == true) {

				fetchOpenGLInfo(OPENGL_TYPE.OpenGL_ES_3);
			} else {

				fetchOpenGLInfo(OPENGL_TYPE.OpenGL_ES_2);
			}

		} catch (Throwable t) {
			// silence
		}
	}

	int clampES3Edition(String version) {

		int numOfNumber = 0;
		String temp = new String();
		Integer glV2 = 0;

		for (int i = 0; i < version.length() && numOfNumber < 2; i++) {

			if (Character.isDigit(version.charAt(i))) {

				temp += version.charAt(i);
				numOfNumber++;
			}
		}

		if (temp.length() > 0) {

			glV2 = Integer.parseInt(temp);
		}

		return glV2;
	}

	private boolean hasES3(String version) {

		boolean hasES3 = false;

		if (clampES3Edition(version) >= 30) {

			hasES3 = true;
		}

		return hasES3;
	}

	public String getData(String key) {
		
		String value = new String();
		if (mData != null && mData.size() > 0) {
			
			value = mData.get(0).get(key);
			if (value == null) {

				value = new String();
			}
		}
		return value;
	}

	class EGLs {
		boolean isValid;
		EGL10 egl;
		EGLDisplay eglDisplay;
		EGLConfig eglSelectedConfig;
		EGLContext eglContext;
		EGLSurface eglSurface;

		GL10 getGL() {

			GL10 gl = null;
			if (eglContext != null) {

				gl = (GL10) eglContext.getGL();
			}
			return gl;
		}
	}

	EGLs createContext(OPENGL_TYPE type) {

		EGLs s = new EGLs();
		int[] version = new int[2];
		final int EGL_OPENGL_ES2_BIT = 4;
		final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

		int[] eglConfigAttributes = null;
		int[] eglContextClientVersion = null;
		switch (type) {

			case OpenGL_ES_3:
				eglConfigAttributes = new int[] { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
				eglContextClientVersion = new int[] { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
				break;
			default:
				eglConfigAttributes = new int[] { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
				eglContextClientVersion = new int[] { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
				break;
		}

		EGLConfig[] matchingEGLConfigs;

		s.egl = (EGL10) EGLContext.getEGL();
		if (s.egl != null) {

			s.eglDisplay = s.egl.eglGetDisplay(EGL_DEFAULT_DISPLAY);
			if (s.eglDisplay != null) {

				boolean retvalue = s.egl.eglInitialize(s.eglDisplay, version);
				if (retvalue) {

					int[] numConfig = new int[1];
					s.egl.eglChooseConfig(s.eglDisplay, eglConfigAttributes, null, 0, numConfig);
					int configSize = numConfig[0];
					if (configSize > 0) {

						matchingEGLConfigs = new EGLConfig[configSize];
						s.egl.eglChooseConfig(s.eglDisplay, eglConfigAttributes, matchingEGLConfigs, configSize, numConfig);

						s.eglSelectedConfig = matchingEGLConfigs[0];
						if (s.eglSelectedConfig != null) {

							s.eglContext = s.egl.eglCreateContext(s.eglDisplay, s.eglSelectedConfig, EGL_NO_CONTEXT, eglContextClientVersion);
							if (s.eglContext != EGL10.EGL_NO_CONTEXT) {

								s.eglSurface = s.egl.eglCreatePbufferSurface(s.eglDisplay, s.eglSelectedConfig, new int[] { EGL_WIDTH, 1, EGL_HEIGHT, 1,
										EGL_NONE });
								if (s.eglSurface != null) {

									s.egl.eglMakeCurrent(s.eglDisplay, s.eglSurface, s.eglSurface, s.eglContext);
									if (s.egl.eglGetError() == EGL_SUCCESS) {

										s.isValid = true;
										return s;
									}
								}
							}
						}
					}
				}
			}
		}

		return s;
	}

	private GLInfo getGLInformation(OPENGL_TYPE type) {

		GLInfo info = null;
		EGLs s = null;

		s = createContext(type);
		if (s != null && s.isValid == true) {

			info = new GLInfo();
			info.vendor = s.getGL().glGetString(GL10.GL_VENDOR);
			info.renderer = s.getGL().glGetString(GL10.GL_RENDERER);
			info.version = s.getGL().glGetString(GL10.GL_VERSION);
			freeEGL(s.eglDisplay, s.eglSurface, s.eglContext, s.egl);
		}

		return info;
	}

	/*
	 * Parameters: OpenGL_ES_3, OpenGL_ES_2
	 * default type is OpenGL_ES_2
	 */
	private boolean fetchOpenGLInfo(OPENGL_TYPE type) {
		int[] version = new int[2];
		final int EGL_OPENGL_ES2_BIT = 4;
		final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

		int[] eglConfigAttributes = null;
		int[] eglContextClientVersion = null;
		switch (type) {
			case OpenGL_ES_3:
				eglConfigAttributes = new int[] { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
				eglContextClientVersion = new int[] { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
				break;
			default:
				eglConfigAttributes = new int[] { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
				eglContextClientVersion = new int[] { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
				break;
		}

		EGL10 egl = (EGL10) EGLContext.getEGL();
		EGLDisplay display = null;
		EGLConfig[] matchingEGLConfigs;
		EGLConfig selectedEGLConfig;
		EGLContext eglContext = null;
		EGLSurface eglSurface = null;

		ArrayList<Integer> eglAttribs = new ArrayList<Integer>();
		eglAttribs.add(EGL10.EGL_CONFIG_ID);
		eglAttribs.add(EGL10.EGL_BUFFER_SIZE);
		eglAttribs.add(EGL10.EGL_ALPHA_SIZE);
		eglAttribs.add(EGL10.EGL_BLUE_SIZE);
		eglAttribs.add(EGL10.EGL_GREEN_SIZE);
		eglAttribs.add(EGL10.EGL_RED_SIZE);
		eglAttribs.add(EGL10.EGL_DEPTH_SIZE);
		eglAttribs.add(EGL10.EGL_STENCIL_SIZE);
		eglAttribs.add(EGL10.EGL_CONFIG_CAVEAT);
		eglAttribs.add(EGL10.EGL_LEVEL);
		eglAttribs.add(EGL10.EGL_MAX_PBUFFER_HEIGHT);
		eglAttribs.add(EGL10.EGL_MAX_PBUFFER_PIXELS);
		eglAttribs.add(EGL10.EGL_MAX_PBUFFER_WIDTH);
		eglAttribs.add(EGL10.EGL_NATIVE_RENDERABLE);
		eglAttribs.add(EGL10.EGL_NATIVE_VISUAL_ID);
		eglAttribs.add(EGL10.EGL_NATIVE_VISUAL_TYPE);
		eglAttribs.add(EGL10.EGL_SAMPLES);
		eglAttribs.add(EGL10.EGL_SAMPLE_BUFFERS);
		eglAttribs.add(EGL10.EGL_SURFACE_TYPE);
		eglAttribs.add(EGL10.EGL_TRANSPARENT_TYPE);
		eglAttribs.add(EGL10.EGL_TRANSPARENT_BLUE_VALUE);
		eglAttribs.add(EGL10.EGL_TRANSPARENT_GREEN_VALUE);
		eglAttribs.add(EGL10.EGL_TRANSPARENT_RED_VALUE);
		eglAttribs.add(EGL10.EGL_RENDERABLE_TYPE);

		HashMap<String, String> data = new HashMap<String, String>();
		data.put("request", type.toString());

		if (egl != null) {
			display = egl.eglGetDisplay(EGL_DEFAULT_DISPLAY);
			if (display != null) {
				boolean retvalue = egl.eglInitialize(display, version);
				if (retvalue) {
					int[] numConfig = new int[1];
					egl.eglChooseConfig(display, eglConfigAttributes, null, 0, numConfig);
					int configSize = numConfig[0];
					if (configSize > 0) {
						matchingEGLConfigs = new EGLConfig[configSize];
						egl.eglChooseConfig(display, eglConfigAttributes, matchingEGLConfigs, configSize, numConfig);

						String eglConfigs = "";
						int[] rvalue = new int[1];
						for (int i = 0; i < configSize; i++) {
							String config = "";
							for (int h = 0; h < eglAttribs.size(); h++) {
								egl.eglGetConfigAttrib(display, matchingEGLConfigs[i], eglAttribs.get(h), rvalue);
								config += "" + rvalue[0] + " ";
							}
							eglConfigs += config + ";";
						}
						data.put("EGL_CONFIGS", eglConfigs);

						selectedEGLConfig = matchingEGLConfigs[0];
						if (selectedEGLConfig != null) {
							eglContext = egl.eglCreateContext(display, selectedEGLConfig, EGL_NO_CONTEXT, eglContextClientVersion);
							if (eglContext != EGL10.EGL_NO_CONTEXT) {
								eglSurface = egl.eglCreatePbufferSurface(display, selectedEGLConfig, new int[] { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE });
								if (eglSurface != null) {
									egl.eglMakeCurrent(display, eglSurface, eglSurface, eglContext);
									GL10 gl = (GL10) eglContext.getGL();
									if (egl.eglGetError() == EGL_SUCCESS) {
										data.put("GL_VENDOR", gl.glGetString(GL10.GL_VENDOR));
										data.put("GL_RENDERER", gl.glGetString(GL10.GL_RENDERER));
										data.put("GL_VERSION", gl.glGetString(GL10.GL_VERSION));
										data.put("GL_EXTENSIONS", gl.glGetString(GL10.GL_EXTENSIONS));

										data.put("EGL_VENDOR", egl.eglQueryString(display, EGL_VENDOR));
										data.put("EGL_EXTENSIONS", egl.eglQueryString(display, EGL_EXTENSIONS));
										data.put("EGL_VERSION", egl.eglQueryString(display, EGL_VERSION));

										data.put("GLES_VERSION_MAJOR", Integer.toString(eglContextClientVersion[1]));

										new InfoGL(gl, data).update();
										mData.add(data);
										freeEGL(display, eglSurface, eglContext, egl);
										return true;
									}
								}
							}
						}
					}
				}
			}
		}
		freeEGL(display, eglSurface, eglContext, egl);
		return false;

	}

	private void freeEGL(EGLDisplay display, EGLSurface eglSurface, EGLContext eglContext, EGL10 egl) {
		
		if (display != null) {
			if (egl != null) {
				egl.eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

				if (eglContext != null) {
					egl.eglDestroyContext(display, eglContext);
					eglContext = null;
				}

				if (eglSurface != null) {
					egl.eglDestroySurface(display, eglSurface);
					eglSurface = null;
				}
				egl.eglTerminate(display);
				display = EGL_NO_DISPLAY;
			}
		}
	}
}
