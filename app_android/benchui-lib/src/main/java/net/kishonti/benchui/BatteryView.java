/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class BatteryView extends View {

	private final Paint bgPaint = new Paint();
	private final Paint fgPaint = new Paint();
	private final Paint batteryPaint = new Paint();
	private final Paint scorePaint = new Paint();
	private final Paint bottomTextPaint = new Paint();
	private final Paint leftTextPaint = new Paint();
	private final Paint rightTextPaint = new Paint();

	private String batteryStr = "Battery";
	private String scoreStr = "Score";

	private float[] battery;
	private float maxBattery = 100;
	private float[] score;
	private float maxScore = 0;
	private int fulltime = 0;

	public BatteryView(Context context) {
		super(context);
	}

	public BatteryView(Context context, AttributeSet attrs) {
		super(context, attrs);
		bgPaint.setStyle(Paint.Style.FILL);
		bgPaint.setColor(0xFFFFFFFF);
		fgPaint.setStrokeWidth(1);
		fgPaint.setStyle(Paint.Style.STROKE);
		fgPaint.setColor(0xFF000000);
		batteryPaint.setStrokeWidth(3);
		batteryPaint.setStyle(Paint.Style.FILL_AND_STROKE);
		batteryPaint.setColor(0xFF4F820E);
		scorePaint.setStrokeWidth(3);
		scorePaint.setStyle(Paint.Style.FILL_AND_STROKE);
		scorePaint.setColor(0xFF6FBDDC);
		bottomTextPaint.setTextAlign(Paint.Align.CENTER);
		bottomTextPaint.setColor(0xFF000000);
		bottomTextPaint.setTextSize(20);
		leftTextPaint.setTextAlign(Paint.Align.RIGHT);
		leftTextPaint.setColor(0xFF000000);
		leftTextPaint.setTextSize(20);
		rightTextPaint.setTextAlign(Paint.Align.LEFT);
		rightTextPaint.setColor(0xFF000000);
		rightTextPaint.setTextSize(20);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		int w = canvas.getWidth();
		int h = canvas.getHeight();
		int marginw = w / 12;
		int marginh = h / 10;
		int tickLength = 10;
		canvas.drawRect(0, 0, w, h, bgPaint);
		for (int i = 0; i < 11; i++) {
			canvas.drawLine(marginw + i * (w - 2 * marginw) / 10, marginh, marginw + i * (w - 2 * marginw) / 10, h - marginh, fgPaint); // fuggoleges vonasok
		}
		canvas.drawLine(marginw, h - marginh, w - marginw, h - marginh, fgPaint); // vizszintes vonal
		for (int i = 0; i < 5; i++) {
			canvas.drawLine(marginw - tickLength / 2, marginh + i * (h - 2 * marginh) / 4, marginw + tickLength / 2, marginh + i * (h - 2 * marginh) / 4, fgPaint); // vizszintes vonasok baloldalt
			canvas.drawLine(w - marginw - tickLength / 2, marginh + i * (h - 2 * marginh) / 4, w - marginw + tickLength / 2, marginh + i * (h - 2 * marginh) / 4, fgPaint); // vizszintes vonasok jobboldalt
		}
		if (score == null) return;
		for (int i = 1; i < score.length; i++) {
			canvas.drawLine(
					marginw + (i - 1) * (w - marginw * 2) / (score.length - 1),
					h - marginh - (h - 2 * marginh) * score[i - 1] / maxScore,
					marginw + i * (w - marginw * 2) / (score.length - 1),
					h - marginh - (h - 2 * marginh) * score[i] / maxScore,
					scorePaint);
			canvas.drawLine(
					marginw + (i - 1) * (w - marginw * 2) / (score.length - 1),
					h - marginh - (h - 2 * marginh) * battery[i - 1] / maxBattery,
					marginw + i * (w - marginw * 2) / (score.length - 1),
					h - marginh - (h - 2 * marginh) * battery[i] / maxBattery,
					batteryPaint);
		}

		// BatteryLegend: left bottom
		// ScoreLegend: right bottom
		float legendmargin = marginh / 9;
		float legendheight = marginh / 3;
		float legendwidth = legendheight * 3;
		leftTextPaint.setTextSize(marginh / 3);
		rightTextPaint.setTextSize(marginh / 3);

		canvas.drawRect(
				marginw + legendmargin,
				h - marginh - legendmargin - legendheight,
				marginw + legendmargin + legendwidth,
				h - marginh - legendmargin, batteryPaint);
		canvas.drawRect(
				w - marginw - legendmargin - legendwidth,
				h - marginh - legendmargin - legendheight,
				w - marginw - legendmargin,
				h - marginh - legendmargin, scorePaint);
		canvas.drawText(batteryStr, marginw + legendmargin * 2 + legendwidth, h - marginh - legendmargin, rightTextPaint);
		canvas.drawText(scoreStr, w - marginw - legendmargin * 2 - legendwidth, h - marginh - legendmargin, leftTextPaint);
		canvas.drawText("100%", marginw - 15, marginh + leftTextPaint.getTextSize() / 2, leftTextPaint);
		canvas.drawText("50%", marginw - 15, h - marginh - (h - 2 * marginh) / 2 + leftTextPaint.getTextSize() / 2, leftTextPaint);
		canvas.drawText(String.format("%.1f", maxScore), w - marginw + 15, marginh + rightTextPaint.getTextSize() / 2, rightTextPaint);
		canvas.drawText(String.format("%.1f", maxScore / 2), w - marginw + 15, h - marginh - (h - 2 * marginh) / 2 + rightTextPaint.getTextSize() / 2, rightTextPaint);

		bottomTextPaint.setTextSize(marginh / 2);
		canvas.drawText("0 min", marginw, h - marginh / 2, bottomTextPaint);
		canvas.drawText(((int) fulltime) + " min", w - marginw, h - marginh / 2, bottomTextPaint);
	}

	public void setText(String batteryStr, String scoreStr) {
		this.batteryStr = batteryStr;
		this.scoreStr = scoreStr;
	}

	public void setData(float[] battery, float[] score, int fulltime) {
		if (battery == null || score == null) {
			clearData();
			return;
		}
		int size = Math.min(battery.length - 1, score.length - 1);
		if (size < 2) {
			clearData();
			return;
		}

		this.battery = new float[size];
		this.score = new float[size];

		this.maxScore = 0;

		for (int i = 0; i < size; i++) {
			this.battery[i] = battery[i + 1];
			this.score[i] = score[i + 1];
			if (this.score[i] > maxScore) maxScore = this.score[i];
		}

		this.fulltime = fulltime / 1000 / 60;

		invalidate();
	}

	private void clearData() {
		this.battery = null;
		this.score = null;
		this.maxScore = 0;
		fulltime = 0;
		invalidate();
	}
}
