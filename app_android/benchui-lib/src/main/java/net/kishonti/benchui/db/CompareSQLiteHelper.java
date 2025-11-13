/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.db;

import net.kishonti.swig.CompareResult;
import net.kishonti.swig.CompareResultVector;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class CompareSQLiteHelper extends SQLiteOpenHelper {
	
	public static final String TABLE_DEVICE = "device";
	public static final String COLUMN_DEVICE_ID = "_id";
	public static final String COLUMN_DEVICE_NAME = "name";
	public static final String COLUMN_DEVICE_VENDOR = "vendor";
	public static final String COLUMN_DEVICE_OS = "os";
	public static final String COLUMN_DEVICE_GPU = "gpu";
	public static final String COLUMN_DEVICE_ARCHITECTURE = "architecture";
	public static final String COLUMN_DEVICE_IMAGE = "image";
	public static final String COLUMN_DEVICE_PU = "pu";
	public static final String COLUMN_DEVICE_FORM_FACTOR = "form_factor";
	
	public static final String TABLE_HARDWARE = "hardware";
	public static final String COLUMN_HARDWARE_ID = "_id";
	public static final String COLUMN_HARDWARE_NAME = "name";

	public static final String TABLE_RESULT = "result";
	public static final String COLUMN_RESULT_ID = "_id";
	public static final String COLUMN_RESULT_DEVICE_ID = "device_id";
	public static final String COLUMN_RESULT_TEST_ID = "test_id";
	public static final String COLUMN_RESULT_SCORE = "score";
	public static final String COLUMN_RESULT_FPS = "fps";
	public static final String COLUMN_RESULT_SCORE_MAX = "score_max";
	public static final String COLUMN_RESULT_FPS_MAX = "fps_max";
	public static final String COLUMN_RESULT_OS = "os";
	public static final String COLUMN_RESULT_API = "api";
	public static final String COLUMN_RESULT_HW_TYPE = "hw_type";
	public static final String COLUMN_RESULT_VARIANT = "variant";
	public static final String COLUMN_RESULT_TEST_BASE = "test_base";
	public static final String COLUMN_RESULT_SUB_TYPE = "sub_type";
	public static final String COLUMN_RESULT_TEST_FAMILY = "test_family";
	public static final String COLUMN_RESULT_HW_NAME_ID = "hw_name_id";

	public static final String TABLE_TEST = "test";
	public static final String COLUMN_TEST_ID = "_id";
	public static final String COLUMN_TEST_STRING_ID = "string_id";
	public static final String COLUMN_TEST_SUBRESULT_ID = "subresult_id";
	public static final String COLUMN_TEST_NAME = "name";
	public static final String COLUMN_TEST_METRIC = "metric";
	public static final String COLUMN_TEST_OFFSCREEN = "offscreen";
	public static final String COLUMN_TEST_LOWERBETTER = "lowerbetter";
	public static final String COLUMN_TEST_TEST_BASE = "test_base";
	public static final String COLUMN_TEST_SUB_TYPE = "sub_type";
	public static final String COLUMN_TEST_TEST_FAMILY = "test_family";
	
	public static enum FormFactorFilter {
		MOBILE,
		DESKTOP,
		ALL
	}

	private static final int DATABASE_VERSION = 2;
	
	private SQLiteDatabase mDatabase; 
	private Context mContext;
	private String mPath;
	
	public CompareSQLiteHelper(Context context, String path) {
		super(context, path, null, DATABASE_VERSION);
		mContext = context;
		mPath = path;
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
		Log.e(CompareSQLiteHelper.class.getName(), "Top-results database creation skipped.");
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
	}
	
	public void open() throws SQLException {
		try{
			mDatabase = SQLiteDatabase.openDatabase(mPath, null, SQLiteDatabase.NO_LOCALIZED_COLLATORS | SQLiteDatabase.CREATE_IF_NECESSARY);
		} catch(Exception ex) {
			Log.d(CompareSQLiteHelper.class.getName(), "Compare database error: " + ex.toString());
		}
	}
	
	public CompareResultVector getCompareListFor(String baseId, String variant, String subType, FormFactorFilter formFilter, String searchFilter) {
		CompareResultVector vector = new CompareResultVector();
		
		try {
		
			String fFilter = " ";
		    if(formFilter == FormFactorFilter.MOBILE) {
		    	fFilter = " AND d.form_factor LIKE '%%mobile%%' ";
		    } else if (formFilter == FormFactorFilter.DESKTOP) {
		    	fFilter = " AND d.form_factor == 'desktop' ";
		    }
		    
		    String sFilter = " ";
		    if(!searchFilter.equals(""))
		    	sFilter = " AND d.name LIKE '%%" + searchFilter + "%%' ";
		    
		    String query = 
			"SELECT " +
		        "d.name, d.vendor, d.gpu, d.architecture, d.image, " +
		        "r.score, r.fps, r.variant, r.test_base, r.sub_type, r.os, r.api, r.test_family " +
		    "FROM result r " +
		    "LEFT JOIN device d ON r.device_id == d._id " +
		    "LEFT JOIN test t ON r.test_id == t._id " +
		    "WHERE r.test_base == '" + baseId + "' " + 
		    		"AND r.variant == '" + variant + "' " + 
		    		"AND r.sub_type == '" + subType + "'" + 
		    		fFilter + 
		    		sFilter + " " +
		    "ORDER BY r.score DESC";
			
			Cursor cursor = mDatabase.rawQuery(query, null);
			
			cursor.moveToFirst();
		    while (!cursor.isAfterLast()) {
		    	CompareResult r = compareResultFromCursor(cursor);
		    	vector.add(r);
		    	cursor.moveToNext();
		    }
			
			cursor.close();
		
		} catch (Exception ex) {
			Log.d(CompareSQLiteHelper.class.getName(), "Compare database error: " + ex.toString());
		}
		
		return vector;
	}
	
	public CompareResultVector getResultsForDevice(String device) {
		CompareResultVector vector = new CompareResultVector();
	    
		try {
		    String query =
				"SELECT " +
		    		"d.name, d.vendor, d.gpu, d.architecture, d.image, " +
		    		"r.score, r.fps, r.variant, r.test_base, r.sub_type, r.os, r.api, r.test_family " +
		        "FROM result r " +
		        "LEFT JOIN device d ON r.device_id == d._id " +
		        "LEFT JOIN test t ON r.test_id == t._id " + 
		        "WHERE d.name == '" + device + "' ";
			
			Cursor cursor = mDatabase.rawQuery(query, null);
			
			cursor.moveToFirst();
		    while (!cursor.isAfterLast()) {
		    	CompareResult r = compareResultFromCursor(cursor);
		    	vector.add(r);
		    	cursor.moveToNext();
		    }
			
			cursor.close();
		} catch (Exception ex) {
			Log.d(CompareSQLiteHelper.class.getName(), "Compare database error: " + ex.toString());
		}
		
		return vector;
	}
	
	private CompareResult compareResultFromCursor(Cursor cursor) {
		CompareResult result = new CompareResult();
		
		String api = cursor.getString(11);
		String architecture = cursor.getString(3);
		String deviceImage = cursor.getString(4);
		String deviceName = cursor.getString(0);
		double fps = cursor.getDouble(6);
		String gpu = cursor.getString(2);
		String os = cursor.getString(10);
		double score = cursor.getDouble(5);
		String subType = cursor.getString(9);
		String testBase = cursor.getString(8);
		String testFamily = cursor.getString(12);
		String variant = cursor.getString(7);
		String vendor = cursor.getString(1);
		
		result.setApi(api != null ? api : "");
		result.setArchitecture(architecture != null ? architecture : "");
		result.setDeviceImage(deviceImage != null ? deviceImage : "");
		result.setDeviceName(deviceName != null ? deviceName : "");
		result.setFps(fps);
		result.setGpu(gpu != null ? gpu : "");
		result.setOs(os != null ? os : "");
		result.setScore(score);
		result.setSubType(subType != null ? subType : "");
		result.setTestBase(testBase != null ? testBase : "");
		result.setTestFamily(testFamily != null ? testFamily : "");
		result.setVariant(variant != null ? variant : "");
		result.setVendor(vendor != null ? vendor : "");
		
		return result;
	}

}
