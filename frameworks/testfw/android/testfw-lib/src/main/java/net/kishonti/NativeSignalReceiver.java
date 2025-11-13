/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti;

public class NativeSignalReceiver {
	public interface Handler {
		public void nativeSignalReceived(int signal);
	}
	public static void nativeSignalReceived(int signal) {
		System.err.println("Native received signal: " + signal);
		if (theHandler != null) {
			theHandler.nativeSignalReceived(signal);
		}
	}
	public static Handler theHandler = null;
}
