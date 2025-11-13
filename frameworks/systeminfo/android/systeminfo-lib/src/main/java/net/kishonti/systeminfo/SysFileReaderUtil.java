/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import android.util.Log;

public class SysFileReaderUtil {

	public static ArrayList<String> catFile(String fileName) {
		
		ArrayList<String> lines = new ArrayList<String>();

		try {
			File file = new File(fileName);
			if (file.exists() && file.canRead()) {
				BufferedReader br = new BufferedReader(new FileReader(file));
				String line;
				while ((line = br.readLine()) != null) {
					
					lines.add(line);
				}
				if (br != null) {
					br.close();
				}
			} else {
				Log.e("gfxb", "can't read file: " + fileName + " (" + file.exists() + " - " + file.canRead() + ")");
			}
		} catch (IOException e) {
			Log.e("gfxb", "exception", e);
		}

		return lines;
	}

	public static String fetchInfo(ArrayList<String> list, String param, char delim) {
		String result = null;
		for (int i = 0; (result == null) && i < list.size(); i++) {
			String line = list.get(i);
			if (line.startsWith(param)) {
				int index = line.indexOf(delim, param.length());
				if ((index > -1) && (index < line.length() - 1)) {
					String value = line.substring(index + 1).trim();
					result = value;
				}
			}
		}
		return result;
	}

	public static String flatten(ArrayList<String> array) {
		
		StringBuilder sb = new StringBuilder();
		for (int i = 0; (array != null) && (i < array.size()); i++) {
			sb.append(array.get(i)).append("\n");
		}
		return sb.toString();
	}

	public static String loadSmallTextFile(String path) {
		
		if (new File(path).exists()) {
			try {
				StringBuilder sb = new StringBuilder();
				BufferedReader br = new BufferedReader(new FileReader(path));
				String line = br.readLine();
				while (line != null) {
					sb.append(line).append('\n');
					line = br.readLine();
				}
				br.close();
				return sb.toString();
			} catch (Exception ex) {
				ex.printStackTrace();
				return "";
			}
		}
		return "";
	}

}
