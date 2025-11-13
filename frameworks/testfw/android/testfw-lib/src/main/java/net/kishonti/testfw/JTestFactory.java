/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.lang.reflect.Constructor;

public class JTestFactory {
	private String mName;
	private Constructor<?> mConstructor = null;

	public JTestFactory(String name) {
		mName = name;
		try {
			Class<?> clazz = Class.forName(mName);
			mConstructor = clazz.getConstructor();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (NoSuchMethodException e) {
			e.printStackTrace();
		}

	}

	public JTestInterface create_test() {
		JTestInterface instance = null;
		try {
			Object object = mConstructor.newInstance();
			if (object instanceof JTestInterface) {
				instance = (JTestInterface) object;
				instance.setName(mName);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return instance;
	}

	public boolean valid() {
		return mConstructor != null; 
	}
}
