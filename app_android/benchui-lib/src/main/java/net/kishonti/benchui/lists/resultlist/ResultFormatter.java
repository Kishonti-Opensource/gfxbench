/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import android.database.Cursor;
import net.kishonti.swig.Result;

public class ResultFormatter {
	
	//TODO create real formatting here with resultItem, error...
	
	public static class FormattedResult {
		private String score;
		private String unit;

		public FormattedResult(String score, String unit) {
			this.score = score;
			this.unit = unit;
		}

		public String getScore() {
			return score;
		}

		public String getUnit() {
			return unit;
		}
	}

	// This method just returns a new FormattedResult instance with its score and unit set to the given values
	protected FormattedResult getFormattedResult(String score, String unit) {
		return new FormattedResult(score, unit);
	}

	// Create formatted result from a Result object. Override this. (Used on result page)
	public FormattedResult getFormattedResult(Result result) {
		return getFormattedResult(result.score(), result.unit());
	}

	// Create formatted result from a database row. Override this. (Used on compare/duel pages)
	public FormattedResult getFormattedResult(Cursor result, String unit) {
		return getFormattedResult(result.getDouble(result.getColumnIndex("score")), unit);
	}

	public FormattedResult getFormattedResult(double score, String unit) {
		int frac = Math.max(0, 3 - (int) Math.floor(Math.log10(score)));
		return getFormattedResult(String.format("%,." + frac + "f", score), unit);
	}

}
