/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

import android.annotation.SuppressLint;
import android.os.Environment;
import android.os.StatFs;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class Memory implements Runnable {

	long totalRam;
	private ArrayList<String> rawMemInfo;

	@Override
	public void run() {
		rawMemInfo = SysFileReaderUtil.catFile("/proc/meminfo");

		String totalMemString = SysFileReaderUtil.fetchInfo(rawMemInfo, "MemTotal", ':');
		try {
			totalMemString = totalMemString.toLowerCase(Locale.ENGLISH).trim();
			long multi = 1L;
			if (totalMemString.endsWith("kb")) {
				totalMemString = totalMemString.substring(0, totalMemString.length() - "kb".length());
				multi = 1024L;
			} else if (totalMemString.endsWith("mb")) {
				totalMemString = totalMemString.substring(0, totalMemString.length() - "mb".length());
				multi = 1024L * 1024L;
			}
			totalRam = Long.parseLong(totalMemString.trim()) * multi;
		} catch (Throwable t) {
			t.printStackTrace();
		}

		determineStorageOptions();
	}

	public static String formatSize(long size) {
		String suffix = null;

		if (size >= 1024) {
			suffix = " KB";
			size /= 1024;
			if (size >= 1024) {
				suffix = " MB";
				size /= 1024;
			}
		}

		StringBuilder resultBuffer = new StringBuilder(Long.toString(size));

		int commaOffset = resultBuffer.length() - 3;
		while (commaOffset > 0) {
			resultBuffer.insert(commaOffset, ',');
			commaOffset -= 3;
		}

		if (suffix != null)
			resultBuffer.append(suffix);
		return resultBuffer.toString();
	}

	// TODO: check source legal status: http://sapienmobile.com/?p=204
	// modified
	private static ArrayList<String> mMounts = new ArrayList<String>();

	public static String[] labels;
	public static String[] paths;
	public static int count = 0;

	public static void determineStorageOptions() {
		setProperties();
	}

	private static Object[][] fileSystems;

	@SuppressWarnings("deprecation")
	@SuppressLint("NewApi")
	private static void setProperties() {

		mMounts.clear();
		try {
			BufferedReader br = new BufferedReader(new FileReader(new File("/system/etc/vold.fstab")));
			if (br != null) {
				String line = null;
				while ((line = br.readLine()) != null) {
					if (line.startsWith("dev_mount")) {
						String[] mountPoint = line.split(" ");
						// Log.e("bu", line);
						mMounts.add(mountPoint[2]);
					}
				}
				br.close();
			}
		} catch (Exception e) {
			// e.printStackTrace();//Hide because on some device has no vold file
		}

		Map<String, String> variables = System.getenv();

		if (Environment.isExternalStorageEmulated() == false) {
			if (variables.get("EXTERNAL_STORAGE") != null) {
				mMounts.add(variables.get("EXTERNAL_STORAGE"));
			}
		}

		if (variables.get("EMULATED_STORAGE_SOURCE") != null) {
			String[] paths = variables.get("EMULATED_STORAGE_SOURCE").split(":");
			for (String path : paths) {
				mMounts.add(path);
			}
		}

		List<String> hashes = new ArrayList<String>();
		for (String mount : mMounts) {
			File path = new File(mount);
			File[] files = path.listFiles();
			if (files != null) {
				String hash = new String(mount + ";");
				for (File file : files) {
					hash += file.getName().hashCode();
					hash += ":";
				}
				hashes.add(hash);
			}
		}

		for (int i = 0; i < hashes.size();) {
			String[] splitted_hashes1 = hashes.get(i).split(";");
			if (splitted_hashes1.length < 2) {
				hashes.remove(i);
				continue;
			}

			for (int h = i + 1; h < hashes.size();) {
				String[] splitted_hashes2 = hashes.get(h).split(";");
				if (splitted_hashes2.length < 2) {
					hashes.remove(h);
					continue;
				}
				if (splitted_hashes1[1].equals(splitted_hashes2[1])) {
					hashes.remove(i);
					continue;
				}
				h++;
			}
			i++;
		}
		mMounts.clear();
		for (int i = 0; i < hashes.size(); i++) {
			String[] splitted_hashes = hashes.get(i).split(";");
			mMounts.add(splitted_hashes[0]);
		}

		mMounts.add(0, Environment.getDataDirectory().getPath());
		fileSystems = new Object[mMounts.size()][];
		for (int i = 0; i < mMounts.size(); i++) {
			String mount = mMounts.get(i);
			File root = new File(mount);

			StatFs stat = new StatFs(root.getPath());
			long blockSize = 0;
			long totalBlocks = 0;
			if (android.os.Build.VERSION.SDK_INT >= 18) {
				blockSize = stat.getBlockSizeLong();
				totalBlocks = stat.getBlockCountLong();
			} else {
				blockSize = stat.getBlockSize();
				totalBlocks = stat.getBlockCount();
			}

			fileSystems[i] = new Object[3];
			fileSystems[i][0] = mount;
			if (i == 0) {
				fileSystems[i][1] = false;
			} else {
				fileSystems[i][1] = true;
			}
			fileSystems[i][2] = Long.valueOf(totalBlocks * blockSize);
		}

		mMounts.clear();
	}

	public int getNumOfStorage() {
		
		if (fileSystems != null) {
			
			return fileSystems.length;
		} else {
			
			return 0;
		}
	}

	public Object[] getData(int i) {
		
		if (fileSystems != null && fileSystems.length > 0) {
			
			return fileSystems[i];
		} else {
			
			return null;
		}
	}

	public long getMemory() {
		
		return totalRam;
	}
}
