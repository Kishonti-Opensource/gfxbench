/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import org.apache.commons.io.IOUtils;

import net.kishonti.benchui.DescriptorFactory;
import net.kishonti.swig.Descriptor;
import android.content.Context;
import android.os.Bundle;

public class AssetDescriptorFactory implements DescriptorFactory {
	private final Context mContext;

	public AssetDescriptorFactory(Context context) {
		mContext = context;
	}

	@Override
	public Descriptor getDescriptorForId(String test_id, Bundle args) {
		try {
			Descriptor desc = new Descriptor();
			String jsonString = IOUtils.toString(mContext.getAssets().open("config/" + test_id + ".json"));
			desc.fromJsonString(jsonString);
			return desc;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}
}
