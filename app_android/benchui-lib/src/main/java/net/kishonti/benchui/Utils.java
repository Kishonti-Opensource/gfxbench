/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import java.io.File;
import java.io.IOException;
import java.util.Date;

import org.apache.commons.io.FileUtils;
import org.json.JSONObject;

import net.kishonti.swig.ApiDefinition;
import net.kishonti.swig.ApiDefinitionVector;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.ResultItem;
import net.kishonti.swig.TestInfo;
import net.kishonti.swig.TestItem;
import net.kishonti.theme.Localizator;
import android.app.UiModeManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.text.format.DateFormat;

public class Utils {

	public static ResultGroup getResultsFromIntent(Intent intent) {
		Bundle bundle = intent.getExtras();
		String jsonResult = bundle.getString("result");
		ResultGroup results = new ResultGroup();
		results.fromJsonString(jsonResult);
		return results;
	}

	public static Intent getIntentForResult(ResultGroup results) {
		Intent intent = new Intent("net.kishonti.testfw.ACTION_TFW_RESULT");
		String str = results.toJsonString();
		intent.putExtra("result", str);
		return intent;
	}

	public static boolean saveResultsToFile(Context context, long session_id, ResultGroup result) {
		try {
			if (BenchmarkApplication.instance.isResultSavingEnabled()) {

				String dirName = DateFormat.format("yyyy_MM_dd_HH_mm_ss", new Date(session_id)).toString();

				File resultDir = new File(BenchmarkApplication.instance.getWorkingDir(), "results/" + dirName);
				resultDir.mkdirs();
				String resultString = result.toJsonString();
				if(result.results().size() > 0) {
					FileUtils.write(new File(resultDir, result.results().get(0).testId() + ".json"), resultString);
				}
			}
		} catch (IOException e) {
			return false;
		}
		return true;
	}

	public static int getLayoutId(Context context, String layout) {
		int id = 0;
		if(Utils.isTablet(context))
			id = context.getResources().getIdentifier(layout + "_tablet", "layout", context.getPackageName());
		if(id == 0)
			id = context.getResources().getIdentifier(layout, "layout", context.getPackageName());

		return id;
	}


	public static boolean isTablet(Context context) {
		boolean ret = context.getResources().getBoolean(R.bool.isTablet);

		UiModeManager uiModeManager = (UiModeManager) context.getSystemService(Context.UI_MODE_SERVICE);
		if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) {
		    ret = true;
		}

		return ret;
	}


	public static boolean isPoneOrTablet(Context context) {
		UiModeManager uiModeManager = (UiModeManager) context.getSystemService(Context.UI_MODE_SERVICE);
		return uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_NORMAL;
	}

	public static String getDescString(Context ctx, TestItem item) {
		String desc = "";

		ApiDefinitionVector graphic_apis = item.testInfo().minimumGraphicsApi();
		ApiDefinitionVector compute_apis = item.testInfo().minimumComputeApi();
		ApiDefinitionVector apis = null;

		boolean compute = ctx.getResources().getString(R.string.app_product_id).contains("compubench");
		if(compute) {
			// no need for compu api
//			if(compute_apis != null && compute_apis.size() > 0) apis = compute_apis;
		} else {
			if(graphic_apis != null && graphic_apis.size() > 0) apis = graphic_apis;
		}

		if(apis != null) {
			for(int i = 0; i < apis.size(); ++i) {
				ApiDefinition def = apis.get(i);
				if(def.type() != ApiDefinition.Type.GL || (compute && def.type() == ApiDefinition.Type.GL)) {
					String api = ApiDefinition.typeToString(def.type()) + " " + def.major() + "." + def.minor();
					desc = appendDescString(desc, api);
				}
			}
		}

		desc = appendDescString(desc, Localizator.getString(ctx, item.description()));


		return desc;
	}

	public static String getDescString(Context ctx, ResultItem item) {
		String desc = "";

		ApiDefinitionVector graphic_apis = item.testInfo().minimumGraphicsApi();
		ApiDefinitionVector compute_apis = item.testInfo().minimumComputeApi();
		ApiDefinitionVector apis = null;

		boolean compute = ctx.getResources().getString(R.string.app_product_id).contains("compubench");
		if(compute) {
			// no need for compu api
//			if(compute_apis != null && compute_apis.size() > 0) apis = compute_apis;
		} else {
			if(graphic_apis != null && graphic_apis.size() > 0) apis = graphic_apis;
		}

		if(apis != null) {
			for(int i = 0; i < apis.size(); ++i) {
				ApiDefinition def = apis.get(i);
				if(def.type() != ApiDefinition.Type.GL || (compute && def.type() == ApiDefinition.Type.GL)) {
					String api = ApiDefinition.typeToString(def.type()) + " " + def.major() + "." + def.minor();
					desc = appendDescString(desc, api);
				}
			}
		}

		if(compute) {
			if(item.resultGroup().results().size() > 0) {
				Result result = item.resultGroup().results().get(0);
				if(result.computeResult() != null) {
					desc = appendDescString(desc, result.computeResult().deviceName());
					desc = appendDescString(desc, result.computeResult().deviceType());
					desc = appendDescString(desc, result.computeResult().deviceVendor());
				}
			}

		} else {
			if(item.resultGroup().results().size() > 0) {
				Result result = item.resultGroup().results().get(0);
				if(result.gfxResult() != null) {
					desc = appendDescString(desc, result.gfxResult().renderer());
					desc = appendDescString(desc, result.gfxResult().surfaceWidth() + " x " + result.gfxResult().surfaceHeight());
				}
			}
		}

		return desc;
	}

	private static String appendDescString(String desc, String appendum) {
		if(desc.equals(""))
			return appendum;
		else
			return desc + " | " + appendum;
	}
}
