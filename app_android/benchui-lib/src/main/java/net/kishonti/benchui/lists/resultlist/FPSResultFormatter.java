/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import android.database.Cursor;

import net.kishonti.swig.Result;

public class FPSResultFormatter extends ResultFormatter {
	
	// We override the public getFormattedResult methods because we need to also pass the fps
	@Override
	public FormattedResult getFormattedResult(Result result) {
		return getFormattedResult(result.score(), result.unit(), (result.gfxResult() != null) ? result.gfxResult().fps() : 0);
	}

	// We override the public getFormattedResult methods because we need to also pass the fps
	@Override
	public FormattedResult getFormattedResult(Cursor result, String unit) {
		return getFormattedResult(result.getDouble(result.getColumnIndex("score")), unit, result.getDouble(result.getColumnIndex("fps")));
	}
	
	@Override
	public FormattedResult getFormattedResult(double score, String unit) {
		return super.getFormattedResult(String.format("%,.0f", score), (unit != null) ? (unit.toLowerCase().contentEquals("frames") ? unit : unit) : null);
	} 

	private FormattedResult getFormattedResult(double score, String unit, double fps) {
		return super.getFormattedResult(String.format("%,.0f", score), (unit != null) ? (unit.toLowerCase().contentEquals("frames") ? "(" + String.format("%.1f", fps) + " fps)" : unit) : null);
	}

}
