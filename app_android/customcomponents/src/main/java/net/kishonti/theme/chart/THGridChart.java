/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme.chart;

import java.util.ArrayList;

import net.kishonti.customcomponents.R;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Path;
import android.graphics.Path.FillType;
import android.graphics.Rect;
import android.graphics.RectF;
import android.support.v4.view.MotionEventCompat;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.MotionEvent;

@SuppressLint("DefaultLocale")
public class THGridChart extends THChart {

	private final float mPadding = 4.0f;
	protected long animStart = 0;
	protected long animDelay = 0;
	protected long animLength = 0;

	private static final Boolean DRAW_DEBUG = false;

	public static class THAxisDivInfo {
		public float firstPos = 0;
		public float lastPos = 1;
		public float divDist = 1;

		public THAxisDivInfo(float first, float last, float div) {
			firstPos = first;
			lastPos = last;
			divDist = div;
		}
	}

	protected final Rect drawedRect = new Rect();

	protected JSONObject mData;

	protected THAxisDivInfo xDivInfo = new THAxisDivInfo(0, 1, 1);
	protected THAxisDivInfo yDivInfo = new THAxisDivInfo(0, 1, 1);

	protected float xMin = Float.MAX_VALUE;
	protected float xMax = Float.MIN_VALUE;
	protected float yMin = Float.MAX_VALUE;
	protected float yMax = Float.MIN_VALUE;

	protected float[] currentMax = new float[]{xMax, yMax};
	protected float[] currentMin = new float[]{xMin, yMin};

	protected float mGridPadding = 30.0f;
	private float mLegendPadding = 10.0f;
	private float mLegendColorIndicatorWidth = 30.0f;

	private String mLeftAxisTitle = "";
	private String mBottomAxisTitle = "";
	protected ArrayList<String> mSampleTitles = new ArrayList<String>();

	/**
	 * Colors
	 */
	private int mLegendColor;
	private int mStrokeColor;
	private int mBorderColor;
	private int mTextColor;

	/**
	 * Paints
	 */
    private Paint mGridBorderPaint;
    private Paint mGridPaint;
    private Paint mLeftAxisPaint;
    private Paint mBottomAxisPaint;
    private Paint mBottomAxisPaintRight;
    private Paint mBottomAxisPaintLeft;
    private Paint mLegendBackPaint;

    protected ArrayList<Paint> mSamplePaints = new ArrayList<Paint>();
    protected ArrayList<Paint> mLegendPaints = new ArrayList<Paint>();

    /**
     * Paths
     */
    private Path mBorderPath;
    private Path mXGridPath;
    private Path mYGridPath;
    protected RectF mGridRect;
    private RectF mLegendRect;
    private RectF mLeftAxisRect;
    private RectF mBottomAxisRect;

    /**
     * Matrix
     */
    protected Matrix mValuesToUnitSpace = new Matrix();
    protected Matrix mUnitToValuesSpace = new Matrix();
    protected Matrix mChartToUnitSpace = new Matrix();
    protected Matrix mUnitToChartSpace = new Matrix();
    protected Matrix mValuesToChartSpace = new Matrix();
    protected Matrix mChartToValuesSpace = new Matrix();

    protected Matrix mTransformMatrix = new Matrix();
    protected Matrix mTransformInverseMatrix = new Matrix();

	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THGridChart(Context context) {
		this(context, null);
	}

	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THGridChart(Context context, AttributeSet attrs) {
		this(context, attrs, R.attr.THGridChartStyle);
	}

	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THGridChart(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);

		final TypedArray array = context.obtainStyledAttributes(
				attrs, R.styleable.THGridChart, defStyle, R.style.THGridChart );


		try {
        	Resources res = getContext().getResources();

            // Retrieve the values from the TypedArray and store into
            // fields of this class.
            mLegendColor = array.getColor(
            		R.styleable.THGridChart_chartLegendColor,
            		res.getColor(R.color.chart_background));
            mStrokeColor = array.getColor(
            		R.styleable.THGridChart_chartStrokeColor,
            		res.getColor(R.color.gridchart_stroke));
            mBorderColor = array.getColor(
            		R.styleable.THGridChart_chartBorderColor,
            		res.getColor(R.color.gridchart_border));
            mTextColor = array.getColor(
            		R.styleable.THGridChart_chartTextColor,
            		res.getColor(R.color.gridchart_text));

        	animDelay = array.getInteger(R.styleable.THGridChart_gridchartAnimDelay, 200);
        	animLength = array.getInteger(R.styleable.THGridChart_gridchartAnimLength, 1000);
        } finally {
            // release the TypedArray so that it can be reused.
            array.recycle();
        }

    	mGridBorderPaint = new Paint();
    	mGridBorderPaint.setColor(mBorderColor);
    	mGridBorderPaint.setStyle(Paint.Style.STROKE);
    	mGridBorderPaint.setStrokeWidth(2.0f);

    	mGridPaint = new Paint();
    	mGridPaint.setColor(mStrokeColor);
    	mGridPaint.setStyle(Paint.Style.STROKE);
    	mGridPaint.setStrokeWidth(1.0f);

    	mBottomAxisPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    	mBottomAxisPaint.setColor(mTextColor);
    	mBottomAxisPaint.setTextSize(
    			TypedValue.applyDimension(
    					TypedValue.COMPLEX_UNIT_SP,
    					10.0f,
    					getResources().getDisplayMetrics()));
    	mBottomAxisPaint.setTextAlign(Align.CENTER);

    	mBottomAxisPaintRight = new Paint(Paint.ANTI_ALIAS_FLAG);
    	mBottomAxisPaintRight.setColor(mTextColor);
    	mBottomAxisPaintRight.setTextSize(
    			TypedValue.applyDimension(
    					TypedValue.COMPLEX_UNIT_SP,
    					10.0f,
    					getResources().getDisplayMetrics()));
    	mBottomAxisPaintRight.setTextAlign(Align.RIGHT);

    	mBottomAxisPaintLeft = new Paint(Paint.ANTI_ALIAS_FLAG);
    	mBottomAxisPaintLeft.setColor(mTextColor);
    	mBottomAxisPaintLeft.setTextSize(
    			TypedValue.applyDimension(
    					TypedValue.COMPLEX_UNIT_SP,
    					10.0f,
    					getResources().getDisplayMetrics()));
    	mBottomAxisPaintLeft.setTextAlign(Align.LEFT);

    	mLeftAxisPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    	mLeftAxisPaint.setColor(mTextColor);
    	mLeftAxisPaint.setTextSize(
    			TypedValue.applyDimension(
    					TypedValue.COMPLEX_UNIT_SP,
    					10.0f,
    					getResources().getDisplayMetrics()));
    	mLeftAxisPaint.setTextAlign(Align.RIGHT);

    	mLegendBackPaint = new Paint();
    	mLegendBackPaint.setColor(mLegendColor);
	}

	@Override
	protected void loadData(String dataString) {
		try {
			JSONObject data = new JSONObject(dataString);
			mData = data;

			mSamplePaints.clear();
			mSampleTitles.clear();
			mLegendPaints.clear();

			JSONArray domain_values = data.getJSONObject("domain").getJSONArray("values");
			for(int i = 0; i < domain_values.length(); ++i) {
				double val = domain_values.getDouble(i);
				if(val > xMax)
					xMax = (float)val;

				if(val < xMin)
					xMin = (float)val;
			}

			JSONArray ranges = data.getJSONArray("values");
			for(int i = 0; i < ranges.length(); ++i) {
				JSONArray range_values = ranges.getJSONObject(i).getJSONArray("values");
				for(int j = 0; j < range_values.length(); ++j) {
					double val = range_values.getDouble(j);
					if(val > yMax)
						yMax = (float)val + 5;

					if(val < yMin)
						yMin = (float)val - 5;
				}

		    	Paint samplePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
				samplePaint.setColor(value_colors[i % 18]);
		    	samplePaint.setStyle(Paint.Style.STROKE);
		    	samplePaint.setStrokeWidth(2.0f);

		    	Paint legendPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
				legendPaint.setColor(value_colors[i % 18]);
		    	legendPaint.setTextSize(
		    			TypedValue.applyDimension(
		    					TypedValue.COMPLEX_UNIT_SP,
		    					10.0f,
		    					getResources().getDisplayMetrics()));
		    	legendPaint.setTextAlign(Align.LEFT);

				mSamplePaints.add(samplePaint);
				mLegendPaints.add(legendPaint);
				mSampleTitles.add(ranges.getJSONObject(i).getString("name"));
			}

			xMax = xMax > xMin ? xMax : xMin + 1;
			yMax = yMax > yMin ? yMax : yMin + 1;

			currentMax = new float[]{xMax, yMax};
			currentMin = new float[]{xMin, yMin};

			mValuesToUnitSpace = new Matrix();
			mValuesToUnitSpace.setTranslate(-xMin, -yMin);
			mValuesToUnitSpace.postScale(1/(xMax - xMin), 1/(yMax - yMin));
			mValuesToUnitSpace.invert(mUnitToValuesSpace);

			mBottomAxisTitle = data.getJSONObject("domain").getString("name");
			mLeftAxisTitle = data.getString("sample_axis");

		} catch (JSONException e) {
			e.printStackTrace();
		}
	}



    /**
     * ------------------------ draw and layout --------------------------------
     */

    /**
     * Do nothing. Do not call the superclass method--that would start a layout pass
     * on this view's children. This lays out its children in onSizeChanged().
     */
    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        // Do nothing. Do not call the superclass method--that would start a layout pass
        // on this view's children. This lays out its children in onSizeChanged().
    }

    /**
     * Calls the layers draw functions.
     */
    @Override
    protected void dispatchDraw(Canvas canvas) {
        super.dispatchDraw(canvas);

        canvas.save();
        canvas.drawPath(mBorderPath, mGridBorderPaint);

        canvas.drawPath(mXGridPath, mGridPaint);
        canvas.drawPath(mYGridPath, mGridPaint);

        drawLeftAxis(canvas);
        drawBottomAxis(canvas);
        drawLegend(canvas);


        if(DRAW_DEBUG) {
            canvas.drawRect(mBottomAxisRect, mGridPaint);
            canvas.drawRect(mLeftAxisRect, mGridPaint);
        }
        canvas.restore();
    }

    /**
 	 * Measurement functions.
     */
    @Override
    protected int getSuggestedMinimumWidth() {
        return (int) 200;
    }

    @Override
    protected int getSuggestedMinimumHeight() {
        return (int) 200;
    }

    /**
     * Measures the required space for the control.
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int minw = getPaddingLeft() + getPaddingRight() + getSuggestedMinimumWidth();
        int w = Math.max(minw, MeasureSpec.getSize(widthMeasureSpec));
        w = Math.min(w, mMaxWidth);

//        int minh = getSuggestedMinimumHeight() + getPaddingBottom() + getPaddingTop();
        int minh = (int)(w * 2.0f/3.0f) + getPaddingBottom() + getPaddingTop();
        int h = Math.max(minh, MeasureSpec.getSize(heightMeasureSpec));
        h = Math.min(h, mMaxHeight);

        setMeasuredDimension(w, h);
    }

    /**
     * Relayout size dependent things.
     */
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        updateBottomAxisRect();
        updateLeftAxisRect();
        updateGridRect();
        updateChartMatrices();
        updateBorderPath();
        updateGridPath();
        updateLegendRect();

        invalidate();
    }


    private void updateChartMatrices() {
		mChartToUnitSpace = new Matrix();
		mChartToUnitSpace.setTranslate(-mGridRect.left, -mGridRect.top);
		float width = mGridRect.right - mGridRect.left > 0 ? mGridRect.right - mGridRect.left : 1;
		float height = mGridRect.bottom - mGridRect.top > 0 ? mGridRect.bottom - mGridRect.top : 1;
		mChartToUnitSpace.postScale(1/width, 1/height);
		mChartToUnitSpace.postScale(1, -1);
		mChartToUnitSpace.postTranslate(0, 1);
		mChartToUnitSpace.invert(mUnitToChartSpace);

		mValuesToChartSpace.set(mValuesToUnitSpace);
		mValuesToChartSpace.postConcat(mUnitToChartSpace);
		mChartToValuesSpace.set(mChartToUnitSpace);
		mChartToValuesSpace.postConcat(mUnitToValuesSpace);

//		String text = getLabelString(currentMax[1]);
//    	mLeftAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
//    	float minDist = drawedRect.width();
//    	text = getLabelString(currentMax[0]);
//    	mBottomAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
//    	minDist = Math.max(drawedRect.width(), minDist);
//    	minDist += mAxisLabelPadding;

    	mBottomAxisPaint.getTextBounds("1.000", 0, "1.000".length(), drawedRect);
		float minDist = drawedRect.width()*2;

		float[] f = new float[9];
		mValuesToChartSpace.getValues(f);
		xDivInfo = getDivision(minDist, currentMax[0], currentMin[0], Math.abs(f[Matrix.MSCALE_X]));
		yDivInfo = getDivision(minDist, currentMax[1], currentMin[1], Math.abs(f[Matrix.MSCALE_Y]));
    }


    private void updateBorderPath() {
        mBorderPath = new Path();
        mBorderPath.setFillType(FillType.EVEN_ODD);
        mBorderPath.moveTo(mGridRect.left, mGridRect.top);
        mBorderPath.lineTo(mGridRect.right, mGridRect.top);
        mBorderPath.lineTo(mGridRect.right, mGridRect.bottom);
        mBorderPath.lineTo(mGridRect.left, mGridRect.bottom);
        mBorderPath.close();
    }

    private void updateGridPath() {

    	mXGridPath = new Path();
    	mYGridPath = new Path();

    	for(float i = xDivInfo.firstPos; i <= xDivInfo.lastPos; i += xDivInfo.divDist) {
    		float[] from = new float[]{i, currentMin[1]};
    		float[] to = new float[]{i, currentMax[1]};

    		mValuesToChartSpace.mapPoints(from);
    		mValuesToChartSpace.mapPoints(to);

        	mXGridPath.moveTo(from[0], from[1]);
        	mXGridPath.lineTo(to[0], to[1]);
    	}

    	for(float i = yDivInfo.firstPos; i <= yDivInfo.lastPos; i += yDivInfo.divDist) {
    		float[] from = new float[]{currentMin[0], i};
    		float[] to = new float[]{currentMax[0], i};

    		mValuesToChartSpace.mapPoints(from);
    		mValuesToChartSpace.mapPoints(to);

        	mYGridPath.moveTo(from[0], from[1]);
        	mYGridPath.lineTo(to[0], to[1]);
    	}
    }

    private void updateLegendRect() {
    	float maxWidth = 0;
    	float height = mLegendPadding;
    	for (int i = 0; i < mSampleTitles.size(); ++i) {
    		String title = mSampleTitles.get(i);
    		Rect titleRect = new Rect();
    		Paint paint = mLegendPaints.get(i);
    		paint.getTextBounds(title, 0, title.length(), titleRect);
    		if(titleRect.width() > maxWidth) maxWidth = titleRect.width();
    		height += Math.max(titleRect.height(), mLegendColorIndicatorWidth) + mLegendPadding;
		}

    	mLegendRect = new RectF(
    			mGridRect.right - maxWidth - mLegendColorIndicatorWidth - 5*mLegendPadding,
    			mGridRect.top + mLegendPadding,
    			mGridRect.right - mLegendPadding,
    			mGridRect.top + mLegendPadding + height);
    }

    private THAxisDivInfo getDivision(float minDist, float maxValue, float minValue, float scale) {
    	float rangeDiff = maxValue - minValue;
    	double division = Math.pow(10, Math.ceil(Math.log10(rangeDiff))) * scale;
    	int rangeMSD = (int)Math.log10(division/minDist);
        double div10 = division / Math.pow(10, rangeMSD);

        int range2MSD = (int)(Math.log10(div10/minDist)/Math.log10(2));
        double div2 = div10 / Math.pow(2, range2MSD);

        double valueMinDist = div2 / scale;
        double firstPos = (Math.floor(minValue / valueMinDist) + 1) * valueMinDist;
        double lastPos = (Math.ceil(maxValue / valueMinDist) - 1) * valueMinDist;

        return new THAxisDivInfo((float)firstPos, (float)lastPos, (float)valueMinDist);
    }

    protected void updateGridRect() {
    	mGridRect = new RectF(
    			mLeftAxisRect.right + 4.0f,
    			mLeftAxisRect.top + mGridPadding,
    			mBottomAxisRect.right - mGridPadding,
    			mBottomAxisRect.top - 4.0f);
    }

    private void updateLeftAxisRect() {
    	String title = mLeftAxisTitle;
    	String max = getLabelString(currentMax[1]);

    	Rect titleRect = new Rect();
    	mLeftAxisPaint.getTextBounds(title, 0, title.length(), titleRect);

    	Rect maxRect = new Rect();
    	mLeftAxisPaint.getTextBounds(max, 0, max.length(), maxRect);

    	mLeftAxisRect = new RectF(
    			mPadding,
    			mPadding,
    			mPadding + titleRect.height() + 2.0f + maxRect.width(),
    			getHeight() - mPadding);
    }

    private void updateBottomAxisRect() {
    	String title = mBottomAxisTitle;
    	String max = getLabelString(currentMax[0]);

    	Rect titleRect = new Rect();
    	mBottomAxisPaint.getTextBounds(title, 0, title.length(), titleRect);

    	Rect maxRect = new Rect();
    	mBottomAxisPaint.getTextBounds(max, 0, max.length(), maxRect);

    	mBottomAxisRect = new RectF(
    			mPadding,
    			getHeight() - (mPadding + titleRect.height() + 2.0f + maxRect.height()),
    			getWidth() - mPadding,
    			getHeight() - mPadding);
    }

    private void drawLeftAxis(Canvas canvas) {
    	for(float i = yDivInfo.firstPos; i <= yDivInfo.lastPos; i += yDivInfo.divDist) {
    		float[] p = new float[]{0, i};
    		mValuesToChartSpace.mapPoints(p);

        	String text = getLabelString(i);
        	mLeftAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);

        	if(p[1] > mGridRect.top + drawedRect.height()*2 && p[1] < mGridRect.bottom - drawedRect.height()*2)
        		canvas.drawText(text, mLeftAxisRect.right, p[1] - (mLeftAxisPaint.descent() + mLeftAxisPaint.ascent()) / 2, mLeftAxisPaint);
    	}

		float[] p = new float[]{0, currentMin[1]};
		mValuesToChartSpace.mapPoints(p);
    	String text = getLabelString(currentMin[1]);
    	mLeftAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
    	canvas.drawText(text, mLeftAxisRect.right, p[1] - drawedRect.height() * 0.5f, mLeftAxisPaint);

		float[] p2 = new float[]{0, currentMax[1]};
		mValuesToChartSpace.mapPoints(p2);
    	text = getLabelString(currentMax[1]);
    	mLeftAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
    	canvas.drawText(text, mLeftAxisRect.right, p2[1] + drawedRect.height(), mLeftAxisPaint);


    	canvas.save();
    	canvas.rotate(-90, mLeftAxisRect.left + drawedRect.height() * 0.7f, p[1] + (p2[1] - p[1])/2.0f);
    	mBottomAxisPaint.getTextBounds(mLeftAxisTitle, 0, mLeftAxisTitle.length(), drawedRect);
    	canvas.drawText(mLeftAxisTitle, mLeftAxisRect.left + drawedRect.height() * 0.7f, p[1] + (p2[1] - p[1])/2.0f, mBottomAxisPaint);
    	canvas.restore();
    }

    private void drawBottomAxis(Canvas canvas) {
    	for(float i = xDivInfo.firstPos; i <= xDivInfo.lastPos; i += xDivInfo.divDist) {
    		float[] p = new float[]{i, 0};
    		mValuesToChartSpace.mapPoints(p);

        	String text = getLabelString(i);
        	mBottomAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
        	if(p[0] > mGridRect.left + drawedRect.width()*2f && p[0] < mGridRect.right - drawedRect.width()*2f)
        		canvas.drawText(text, p[0], mBottomAxisRect.top + drawedRect.height(), mBottomAxisPaint);
    	}

		float[] p = new float[]{currentMin[0], 0};
		mValuesToChartSpace.mapPoints(p);
    	String text = getLabelString(currentMin[0]);
    	mBottomAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
    	canvas.drawText(text, p[0], mBottomAxisRect.top + drawedRect.height(), mBottomAxisPaintLeft);

		float[] p2 = new float[]{currentMax[0], 0};
		mValuesToChartSpace.mapPoints(p2);
    	text = getLabelString(currentMax[0]);
    	mBottomAxisPaint.getTextBounds(text, 0, text.length(), drawedRect);
    	canvas.drawText(text, p2[0], mBottomAxisRect.top + drawedRect.height(), mBottomAxisPaintRight);

    	mBottomAxisPaint.getTextBounds(mBottomAxisTitle, 0, mBottomAxisTitle.length(), drawedRect);
    	canvas.drawText(mBottomAxisTitle, p[0] + (p2[0] - p[0])/2.0f, mBottomAxisRect.bottom, mBottomAxisPaint);

    }

    private void drawLegend(Canvas canvas) {
    	canvas.drawRect(mLegendRect, mLegendBackPaint);

    	float height = mLegendPadding;
    	for (int i = 0; i < mSampleTitles.size(); ++i) {
    		String title = mSampleTitles.get(i);
    		Paint paint = mLegendPaints.get(i);
    		paint.getTextBounds(title, 0, title.length(), drawedRect);

    		float centerY = mLegendRect.top + height + Math.max(drawedRect.height(), mLegendColorIndicatorWidth) *0.5f;
    		height += Math.max(drawedRect.height(), mLegendColorIndicatorWidth) + mLegendPadding;

    		canvas.drawText(
    				title,
    				mLegendRect.left + mLegendPadding + mLegendColorIndicatorWidth + mLegendPadding,
    				centerY - (paint.descent() + paint.ascent()) / 2,
    				paint);

    		drawedRect.left = (int)(mLegendRect.left + mLegendPadding);
    		drawedRect.top = (int)(centerY - mLegendColorIndicatorWidth*0.5f);
    		drawedRect.right = (int)(mLegendRect.left + mLegendPadding + mLegendColorIndicatorWidth);
    		drawedRect.bottom = (int)(centerY + mLegendColorIndicatorWidth*0.5f);
    		canvas.drawRect(drawedRect, mGridBorderPaint);

    		drawedRect.left = drawedRect.left + 4;
    		drawedRect.top = drawedRect.top + 4;
    		drawedRect.right = drawedRect.right - 4;
    		drawedRect.bottom = drawedRect.bottom - 4;
    		canvas.drawRect(drawedRect, paint);
		}
    }

    private void disallowParentInterceptTouchEvent(boolean b) {
        if (getParent() != null) {
        	getParent().requestDisallowInterceptTouchEvent(b);
        }
    }

    protected void innerChartChange() {
    	updateGridPath();
    }

    public String getLabelString(float val) {
		int frac = Math.max(0, 3 - (int) Math.max(0, Math.floor(Math.log10(val))));
		return String.format("%,." + frac + "f", val);
	}






    private int mActivePointerId = MotionEvent.INVALID_POINTER_ID;
    private int mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;

    private float mLastTouch1X = 0;
    private float mLastTouch1Y = 0;
    private float mLastTouch2X = 0;
    private float mLastTouch2Y = 0;


    @Override
    public boolean onTouchEvent(MotionEvent ev) {

    	boolean handled = false;
    	boolean parentDisallow = false;


    	final int action = MotionEventCompat.getActionMasked(ev);
    	switch(action) {
    	case MotionEvent.ACTION_DOWN: {
    		final int pointerIndex = MotionEventCompat.getActionIndex(ev);
    		final float x = MotionEventCompat.getX(ev, pointerIndex);
            final float y = MotionEventCompat.getY(ev, pointerIndex);

            // Remember where we started (for dragging)
            mLastTouch1X = x;
            mLastTouch1Y = y;

            // Save the ID of this pointer (for dragging)
            mActivePointerId = MotionEventCompat.getPointerId(ev, 0);

            handled = true;
            parentDisallow = true;
            break;
    	}
    	case MotionEvent.ACTION_POINTER_DOWN: {
    		final int pointerIndex = MotionEventCompat.getActionIndex(ev);
            final int pointerId = MotionEventCompat.getPointerId(ev, pointerIndex);

            if (pointerId != mActivePointerId) {
            	mSecondaryPointerId = pointerId;
        		final float x = MotionEventCompat.getX(ev, pointerIndex);
                final float y = MotionEventCompat.getY(ev, pointerIndex);
                mLastTouch2X = x;
                mLastTouch2Y = y;
            }

            handled = true;
            parentDisallow = true;
            break;
    	}

		case MotionEvent.ACTION_UP: {
		    mActivePointerId = MotionEvent.INVALID_POINTER_ID;
		    mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;

            handled = true;
            parentDisallow = true;
            break;
		}

		case MotionEvent.ACTION_CANCEL: {
		    mActivePointerId = MotionEvent.INVALID_POINTER_ID;
		    mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;

            handled = true;
            parentDisallow = true;
            break;
		}

	    case MotionEvent.ACTION_POINTER_UP: {
	        final int pointerIndex = MotionEventCompat.getActionIndex(ev);
	        final int pointerId = MotionEventCompat.getPointerId(ev, pointerIndex);

	        if (pointerId == mActivePointerId) {
	            mActivePointerId = MotionEvent.INVALID_POINTER_ID;
	        	if(mSecondaryPointerId != MotionEvent.INVALID_POINTER_ID) {
	        		mActivePointerId = mSecondaryPointerId;
	        		mLastTouch1X = mLastTouch2X;
	        		mLastTouch1Y = mLastTouch2Y;
	        	}
			    mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;
	        } else {
			    mSecondaryPointerId = MotionEvent.INVALID_POINTER_ID;
	        }

            handled = true;
            parentDisallow = true;
            break;
	    }
	    case MotionEvent.ACTION_MOVE: {
	        handled = false;
	        parentDisallow = false;

	        if(mActivePointerId != MotionEvent.INVALID_POINTER_ID &&
	        		mSecondaryPointerId != MotionEvent.INVALID_POINTER_ID) {

	        	parentDisallow = handled = onScale(ev);

	            final int pointerIndex1 = MotionEventCompat.findPointerIndex(ev, mActivePointerId);
	            final int pointerIndex2 = MotionEventCompat.findPointerIndex(ev, mSecondaryPointerId);

	    		final float x1 = MotionEventCompat.getX(ev, pointerIndex1);
	    		final float y1 = MotionEventCompat.getY(ev, pointerIndex1);

	    		final float x2 = MotionEventCompat.getX(ev, pointerIndex2);
	    		final float y2 = MotionEventCompat.getY(ev, pointerIndex2);

	    		mLastTouch1X = x1;
	    		mLastTouch1Y = y1;
	    		mLastTouch2X = x2;
	    		mLastTouch2Y = y2;

	        } else if(mActivePointerId != MotionEvent.INVALID_POINTER_ID) {
	        	parentDisallow = handled = onScroll(ev);

	            final int pointerIndex1 = MotionEventCompat.findPointerIndex(ev, mActivePointerId);

	    		final float x1 = MotionEventCompat.getX(ev, pointerIndex1);
	    		final float y1 = MotionEventCompat.getY(ev, pointerIndex1);

	    		mLastTouch1X = x1;
	    		mLastTouch1Y = y1;
	        }
	        break;
	    }
    	}

    	disallowParentInterceptTouchEvent(parentDisallow);
    	return handled;
	}


	public boolean onScale(MotionEvent ev) {

		Matrix m = getTransformMatrix(ev);

		float[] prevMax = new float[]{currentMax[0], currentMax[1]};
		float[] prevMin = new float[]{currentMin[0], currentMin[1]};

		m.mapPoints(currentMax);
		m.mapPoints(currentMin);

		if( currentMax[0] == xMax &&
			currentMax[1] == yMax &&
			currentMin[0] == xMin &&
			currentMin[1] == yMin) {
			return false;
		}

		if(currentMax[0] - currentMin[0] < 0.1f * (xMax - xMin)) {
			currentMax[0] = prevMax[0];
			currentMin[0] = prevMin[0];
		}
		if(currentMax[1] - currentMin[1] < 0.1f * (yMax - yMin)) {
			currentMax[1] = prevMax[1];
			currentMin[1] = prevMin[1];
		}


		if(currentMax[0] > xMax) currentMax[0] = xMax;
		if(currentMax[1] > yMax) currentMax[1] = yMax;
		if(currentMin[0] < xMin) currentMin[0] = xMin;
		if(currentMin[1] < yMin) currentMin[1] = yMin;

		mValuesToUnitSpace = new Matrix();
		mValuesToUnitSpace.setTranslate(-currentMin[0], -currentMin[1]);
		mValuesToUnitSpace.postScale(1/(currentMax[0] - currentMin[0]), 1/(currentMax[1] - currentMin[1]));
		mValuesToUnitSpace.invert(mUnitToValuesSpace);

		updateChartMatrices();
		innerChartChange();
		invalidate();

		return true;
	}

	public Matrix getTransformMatrix(MotionEvent ev) {
		Matrix m = new Matrix();

        final int pointerIndex1 = MotionEventCompat.findPointerIndex(ev, mActivePointerId);
        final int pointerIndex2 = MotionEventCompat.findPointerIndex(ev, mSecondaryPointerId);

		final float x1 = MotionEventCompat.getX(ev, pointerIndex1);
		final float y1 = MotionEventCompat.getY(ev, pointerIndex1);

		final float x2 = MotionEventCompat.getX(ev, pointerIndex2);
		final float y2 = MotionEventCompat.getY(ev, pointerIndex2);

		final float focusX = x1 + (x2 - x1)*0.5f;
		final float focusY = y1 + (y2 - y1)*0.5f;

		final float currentSpanX = Math.abs(x2 - x1);
		final float currentSpanY = Math.abs(y2 - y1);

		float lastSpanX = Math.abs(mLastTouch2X - mLastTouch1X);
		float lastSpanY = Math.abs(mLastTouch2Y - mLastTouch1Y);

		float[] focus = new float[]{focusX, focusY};
		mChartToValuesSpace.mapPoints(focus);

		float xscale = 1 + (lastSpanX/currentSpanX - 1) * 0.5f;
		float yscale = 1 + (lastSpanY/currentSpanY - 1) * 0.5f;

		if(Float.isNaN(xscale) || Float.isInfinite(xscale)) xscale = 1;
		if(Float.isNaN(yscale) || Float.isInfinite(yscale)) yscale = 1;


		float atanScale = (float)Math.atan(Math.abs(1 - yscale) / Math.abs(1- xscale));

		if(atanScale > Math.PI / 3.0f) xscale = 1;
		if(atanScale < Math.PI / 6.0f) yscale = 1;

		m.setScale(
				xscale,
				yscale,
				focus[0],
				focus[1]);
		return m;
	}

	public boolean onScroll(MotionEvent ev) {
        final int pointerIndex1 = MotionEventCompat.findPointerIndex(ev, mActivePointerId);

		final float x1 = MotionEventCompat.getX(ev, pointerIndex1);
		final float y1 = MotionEventCompat.getY(ev, pointerIndex1);

		float distanceX = mLastTouch1X - x1;
		float distanceY = mLastTouch1Y - y1;

		Matrix m = new Matrix();
		float[] focus = new float[]{distanceX, distanceY};
		mChartToValuesSpace.mapVectors(focus);
		m.setTranslate(focus[0], focus[1]);

		float[] prevMax = new float[]{currentMax[0], currentMax[1]};
		float[] prevMin = new float[]{currentMin[0], currentMin[1]};

		m.mapPoints(currentMax);
		m.mapPoints(currentMin);

		if(currentMax[0] > xMax) {
			currentMax[0] = xMax;
			currentMin[0] = xMax - prevMax[0] + prevMin[0];
		}
		if(currentMax[1] > yMax) {
			currentMax[1] = yMax;
			currentMin[1] =  yMax - prevMax[1] + prevMin[1];
		}
		if(currentMin[0] < xMin) {
			currentMin[0] = xMin;
			currentMax[0] = xMin + prevMax[0] - prevMin[0];
		}
		if(currentMin[1] < yMin) {
			currentMin[1] = yMin;
			currentMax[1] = yMin + prevMax[1] - prevMin[1];
		}


		boolean xBlock = (currentMax[0] == xMax && distanceX >= 0) || (currentMin[0] == xMin && distanceX <= 0);
		boolean yBlock = (currentMax[1] == yMax && distanceY <= 0) || (currentMin[1] == yMin && distanceY >= 0);
		if(xBlock && yBlock) return false;

		mValuesToUnitSpace = new Matrix();
		mValuesToUnitSpace.setTranslate(-currentMin[0], -currentMin[1]);
		mValuesToUnitSpace.postScale(1/(currentMax[0] - currentMin[0]), 1/(currentMax[1] - currentMin[1]));
		mValuesToUnitSpace.invert(mUnitToValuesSpace);

		updateChartMatrices();
		innerChartChange();
		invalidate();

		return true;
	}

}
