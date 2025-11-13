/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

public class ImageFormat {
	public static String toString(Integer format) {
		switch (format) {
		case android.graphics.ImageFormat.UNKNOWN:
			return "UNKNOWN";
		case android.graphics.ImageFormat.RGB_565:
			return "RGB_565";
		case android.graphics.ImageFormat.YV12:
			return "YV12";
		case android.graphics.ImageFormat.NV16:
			return "NV16";
		case android.graphics.ImageFormat.NV21:
			return "NV21";
		case android.graphics.ImageFormat.YUY2:
			return "YUY2";
		case android.graphics.ImageFormat.JPEG:
			return "JPEG";
		case android.graphics.ImageFormat.YUV_420_888:
			return "YUV_420_888";
		case android.graphics.ImageFormat.RAW_SENSOR:
			return "RAW_SENSOR";
		case android.graphics.ImageFormat.RAW10:
			return "RAW10";
		default:
			return Integer.toHexString(format);
		}
	}
}
