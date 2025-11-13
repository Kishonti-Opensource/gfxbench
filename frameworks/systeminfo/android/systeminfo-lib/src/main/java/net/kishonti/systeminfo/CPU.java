/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class CPU implements Runnable {

	private ArrayList<String> rawCpuInfo = new ArrayList<String>();
	private ArrayList<String> rawCpuFreqSrtings = new ArrayList<String>();
	private ArrayList<Integer> freqs;
	private int cpuCores;

	@Override
	public void run() {
		
		rawCpuInfo = fetchCpuInfo();
		freqs = fetchCpuFrequencies(rawCpuFreqSrtings);
	}

	private ArrayList<Integer> fetchCpuFrequencies(ArrayList<String> rawToFill) {

		ArrayList<Integer> result = new ArrayList<Integer>();
		boolean existingCpu = true;
		for (int i = 0; existingCpu && (i < 1000); i++) {
			// not all cpu has detailed info, hence checking only the folder to determine the number
			File folder = new File("/sys/devices/system/cpu/cpu" + i);
			if (folder.exists()) {
				
				cpuCores++;
				File file = new File("/sys/devices/system/cpu/cpu" + i + "/cpufreq/cpuinfo_max_freq");
				if (file.exists()) {
					
					ArrayList<String> contents = SysFileReaderUtil.catFile(file.getAbsolutePath());
					if (contents.size() > 0) {
						
						String freqAsString = contents.get(0); // first line only
						if (rawToFill != null) {
							
							rawToFill.add(freqAsString);
						}
						int freqMHz = 0;
						try {
							freqMHz = Integer.parseInt(freqAsString.trim());
							freqMHz = freqMHz / 1000;
						} catch (Throwable t) {

						}
						result.add(Integer.valueOf(freqMHz));
					} else {
						existingCpu = false;
					}
				} else {
					// existingCpu = false;
				}
			} else {
				existingCpu = false;
			}
		}
		return result;
	}

	private ArrayList<String> fetchCpuInfo() {

		return SysFileReaderUtil.catFile("/proc/cpuinfo");
	}

	public String getRawData() {

		StringBuilder sb = new StringBuilder();
		sb.append("/** CPU **/").append("\n");
		sb.append("android.os.Build.CPU_ABI: ").append(android.os.Build.CPU_ABI).append("\n");
		sb.append("android.os.Build.CPU_ABI2: ").append(android.os.Build.CPU_ABI2).append("\n");
		sb.append("CPU info (/proc/cpuinfo): ").append("\n");
		for (int i = 0; (rawCpuInfo != null) && (i < rawCpuInfo.size()); i++) {
			sb.append(rawCpuInfo.get(i)).append("\n");
		}
		sb.append("CPU freqs (/sys/devices/system/cpu/cpu<N>/cpufreq/cpuinfo_max_freq): ").append("\n");
		for (int i = 0; (rawCpuFreqSrtings != null) && (i < rawCpuFreqSrtings.size()); i++) {
			sb.append(rawCpuFreqSrtings.get(i)).append("\n");
		}
		sb.append("/sys/devices/system/cpu/online: ").append(SysFileReaderUtil.flatten(SysFileReaderUtil.catFile("/sys/devices/system/cpu/online")))
				.append("\n");
		sb.append("/sys/devices/system/cpu/offline: ").append(SysFileReaderUtil.flatten(SysFileReaderUtil.catFile("/sys/devices/system/cpu/offline")))
				.append("\n");
		sb.append("/sys/devices/system/cpu/possible: ").append(SysFileReaderUtil.flatten(SysFileReaderUtil.catFile("/sys/devices/system/cpu/possible")))
				.append("\n");
		sb.append("/sys/devices/system/cpu/present: ").append(SysFileReaderUtil.flatten(SysFileReaderUtil.catFile("/sys/devices/system/cpu/present")))
				.append("\n");

		sb.append("/** END **/").append("\n");

		return sb.toString();
	}

	public Map<String, String> getCPUDataFiles() {

		Map<String, String> result = new HashMap<String, String>();
		String[] fileName = { "/sys/devices/system/cpu/online", "/sys/devices/system/cpu/offline", "/sys/devices/system/cpu/possible",
				"/sys/devices/system/cpu/present", "/proc/cpuinfo" };
		
		for (String name : fileName) {

			result.put(name, SysFileReaderUtil.loadSmallTextFile(name).trim());
		}

		try {
			String cpuMaxNr = SysFileReaderUtil.loadSmallTextFile("/sys/devices/system/cpu/kernel_max").trim();
			int cpuCount = Integer.parseInt(cpuMaxNr) + 1;
			String freqList = SysFileReaderUtil.loadSmallTextFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq").trim();
			for (int i = 1; i < cpuCount; ++i) {

				freqList += "\n" + SysFileReaderUtil.loadSmallTextFile("/sys/devices/system/cpu/cpu" + i + "/cpufreq/cpuinfo_max_freq").trim();
			}
			result.put("/sys/devices/system/cpu/cpufreq/cpuinfo_max_freq", freqList);
		} catch (NumberFormatException e) {

		}

		return result;
	}

	public int getCores() {

		return cpuCores;
	}

	public ArrayList<Integer> getFrequency() {

		return freqs;
	}

}
