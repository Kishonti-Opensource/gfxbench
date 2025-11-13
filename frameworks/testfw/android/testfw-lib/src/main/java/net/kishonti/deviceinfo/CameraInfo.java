/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

import java.util.List;

import android.hardware.Camera.Size;

@SuppressWarnings("deprecation")
public class CameraInfo {
	String facing;
	public int max_exposure_compensation;
	public int max_num_detected_faces;
	public int max_num_focus_areas;
	public int max_num_metering_areas;
	public int min_exposure_compensation;
	public Size preferred_preview_size_for_video;
	public List<Size> supported_picture_sizes;
	public List<Size> supported_jpeg_thumbnail_sizes;
	public List<Size> supported_video_sizes;
	public List<String> supported_focus_modes;
	public List<String> supported_flash_modes;
	public List<String> supported_color_effects;
	public List<String> supported_antibanding;
	public List<int[]> supported_preview_fps_range;
	public List<String> supported_white_balance;
	public List<String> supported_picture_formats;
	public List<String> supported_preview_formats;
}
