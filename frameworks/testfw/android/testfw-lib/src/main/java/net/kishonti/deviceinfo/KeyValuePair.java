/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

public class KeyValuePair<V> {
	public KeyValuePair(String k, V v) {
		key = k;
		value = v;
	}
	public String key;
	public V value;
}
