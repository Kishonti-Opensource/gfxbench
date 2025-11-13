/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.corporate;

import net.kishonti.benchui.AssetDescriptorFactory;
import net.kishonti.swig.Descriptor;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;

public class FileDescriptorFactory extends AssetDescriptorFactory {

	// names are mostly for backward compatibility
	private static final String INTEGER_ARG_NAMES[] = {
		"offscreen_width",
		"offscreen_height",
		"frame_step_time",
		"max_rendered_frames",
		"screenmode",
		"play_time",
		"start_animation_time",
		"single_frame",
		"disabled_render_bits",
		"fps_log_window"
	};
	private static final String BOOLEAN_ARG_NAMES[] = {
		"endless",
		"force_highp",
		"tessellation_enabled"
	};
	private static final String FLOAT_ARG_NAMES[] = {
		"fps_limit",
		"brightness"
	};
	private static final String STRING_ARG_NAMES[] = {
		"screenshot_frames",
		"wg_sizes",
		"texture_type"
	};

	private static final String TAG = "DescriptorFactory";


	public FileDescriptorFactory(Context context) {
		super(context);
	}

	@Override
	public Descriptor getDescriptorForId(String test_id, Bundle args) {
		Descriptor desc = super.getDescriptorForId(test_id, args);
		
		if (args != null && desc!=null) {
			desc.env().compute().setConfigIndex(args.getInt("configIndex", 0));
			
			desc.setRawConfigBool("interop", args.getBoolean("interop", true));
			
			int custom_width = args.getInt("-width", 0);
			int custom_height = args.getInt("-height", 0);
			if(custom_width > 0 && custom_height > 0) {
				desc.setRawConfigBool("virtual_resolution", true);
				desc.setRawConfigDouble("test_width", args.getInt("-width", desc.env().width()));
				desc.setRawConfigDouble("test_height", args.getInt("-height", desc.env().height()));
			}

			updateRawConfigFromBundle(desc, args);

			if (args.containsKey("-fsaa")) {
				int samples = args.getInt("fsaa");
				desc.setRawConfigDouble("fsaa", samples);
				desc.env().graphics().config().setSamples(samples);
			}

			for (String iname : INTEGER_ARG_NAMES) {
				String key = "-" + iname;
				if (args.containsKey(key)) {
					desc.setRawConfigDouble(iname, args.getInt(key));
				}
			}

			for (String bname : BOOLEAN_ARG_NAMES) {
				String key = "-" + bname;
				if (args.containsKey(key)) {
					// backward compatibility handle int argument as bool in descriptor
					desc.setRawConfigBool(bname, args.getInt(key) > 0);
				}
			}

			for (String fname : FLOAT_ARG_NAMES) {
				String key = "-" + fname;
				if (args.containsKey(key)) {
					desc.setRawConfigDouble(fname, args.getFloat(key));
				}
			}

			for (String sname : STRING_ARG_NAMES) {
				String key = "-" + sname;
				if (args.containsKey(key)) {
					desc.setRawConfigString(sname, args.getString(key));
				}
			}

			if (desc.rawConfigb("virtual_resolution", false) && desc.rawConfign("screenmode", 0.0) == 0.0) {
				// Use HW scaler on Android when on-screen mode is specified
				desc.env().setWidth(args.getInt("-width", desc.env().width()));
				desc.env().setHeight(args.getInt("-height", desc.env().height()));
			}
		}

		return desc;
	}

	private void updateRawConfigFromBundle(Descriptor desc, Bundle args) {
		for (String key : args.keySet()) {
			if (key.startsWith("raw_config")) {
				String[] keypath = key.split("\\.", 2);
				if (keypath.length == 2) {
					String keyname = keypath[1];
					Object val = args.get(key);
					if (val instanceof Integer) {
						desc.setRawConfigDouble(keyname, args.getInt(key));
					} else if (val instanceof Float) {
						desc.setRawConfigDouble(keyname, args.getFloat(key));
					} else if (val instanceof Long) {
						desc.setRawConfigDouble(keyname, args.getLong(key));
					} else if (val instanceof Boolean) {
						desc.setRawConfigBool(keyname, args.getBoolean(key));
					} else if (val instanceof String) {
						desc.setRawConfigString(keyname, args.getString(key));
					} else {
						Log.w(TAG, "Type not supported for raw_config: " + val.getClass().getCanonicalName());
					}
				}
			}
		}
	}
}
