/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.model;

import java.util.HashMap;

import android.app.Fragment;

public class BenchmarkModel {
	private HashMap<String, String> fragmentStates = new HashMap<String, String>();
	private HashMap<String, String> fragmentParams = new HashMap<String, String>();
	
	public void SetFragmentState(String fragmentClassName, String state) {
		fragmentStates.put(fragmentClassName, state);
	}
	
	public void SetFragmentState(Fragment fragment, String state) {
		fragmentStates.put(fragment.getClass().toString(), state);
	}
	
	public String getFragmentState(Fragment fragment) {
		String state = fragmentStates.get(fragment.getClass().toString());
		return (state != null) ? state : "";
	}
	
	public void SetFragmentParams(String fragmentClassName, String params) {
		fragmentParams.put(fragmentClassName, params);
	}
	
	public void SetFragmentParams(Fragment fragment, String params) {
		fragmentParams.put(fragment.getClass().toString(), params);
	}
	
	public String getFragmentParams(Fragment fragment) {
		String params = fragmentParams.get(fragment.getClass().toString());
		return (params != null) ? params : "";
	}
}
