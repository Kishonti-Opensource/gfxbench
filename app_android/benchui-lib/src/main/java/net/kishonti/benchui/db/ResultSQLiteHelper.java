/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.db;

import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.ResultGroupVector;
import net.kishonti.swig.Session;
import net.kishonti.swig.SessionVector;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class ResultSQLiteHelper extends SQLiteOpenHelper {
	
	public static final String TABLE_SESSIONS = "Sessions";
	public static final String COLUMN_SESSIONS_SESSION_ID = "session_id";
	public static final String COLUMN_SESSIONS_IS_FINISHED = "is_finished";
	public static final String COLUMN_SESSIONS_CONFIGURATION = "configuration";

	public static final String TABLE_RESULTS = "Results";
	public static final String COLUMN_RESULTS_RESULT_ID = "result_id";
	public static final String COLUMN_RESULTS_SESSION_ID = "session_id";
	public static final String COLUMN_RESULTS_SCORE = "score";
	public static final String COLUMN_RESULTS_RAW = "raw";
	
	public static final String CREATE_SESSIONS = 
			"CREATE TABLE IF NOT EXISTS 'Sessions' (" +
	            "'session_id' INTEGER PRIMARY KEY NOT NULL, " +
	            "'is_finished' BOOLEAN NOT NULL, " +
	            "'configuration' TEXT NOT NULL" +
	        ");";
	
	public static final String CREATE_RESULTS = 
			"CREATE TABLE IF NOT EXISTS 'Results' (" +
	            "'result_id' TEXT NOT NULL, " +
	            "'session_id' INTEGER NOT NULL, " +
	            "'score' TEXT NOT NULL, " +
	            "'raw' TEXT NOT NULL, " +
	            "PRIMARY KEY(result_id, session_id), " +
	            "FOREIGN KEY(session_id) REFERENCES Sessions(session_id)" +
	        ");";

	private static final String DATABASE_NAME = "results.sqlite";
	private static final int DATABASE_VERSION = 2;
	
	private SQLiteDatabase mDatabase; 
	private Context mContext;
	
	public ResultSQLiteHelper(Context context, String path) {
		super(context, path + "/" + DATABASE_NAME, null, DATABASE_VERSION);
		mContext = context;
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
	    db.execSQL(CREATE_SESSIONS);
	    db.execSQL(CREATE_RESULTS);
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		Log.w(ResultSQLiteHelper.class.getName(),
		        "Upgrading database from version " + oldVersion + " to "
		            + newVersion + ", which will destroy all old data");
	    db.execSQL("DROP TABLE IF EXISTS " + TABLE_RESULTS);
	    db.execSQL("DROP TABLE IF EXISTS " + TABLE_SESSIONS);
	    onCreate(db);
	}
	
	public void open() throws SQLException {
		mDatabase = this.getWritableDatabase();
	}
	
	
	
	public void clearResults() {
		mDatabase.delete(TABLE_RESULTS, null, null);
		mDatabase.delete(TABLE_SESSIONS, null, null);
	}
	
	public boolean insertResult(String resultId, long sessionId, double score, String raw) {
		ContentValues values = new ContentValues();
		values.put(COLUMN_RESULTS_RESULT_ID, resultId);
		values.put(COLUMN_RESULTS_SESSION_ID, sessionId);
		values.put(COLUMN_RESULTS_SCORE, score);
		values.put(COLUMN_RESULTS_RAW, raw);
		
		return mDatabase.insert(TABLE_RESULTS, null, values) > -1;
	}
	
	public boolean updateResult(String resultId, long sessionId, double score, String raw) {
		ContentValues values = new ContentValues();
		values.put(COLUMN_RESULTS_RESULT_ID, resultId);
		values.put(COLUMN_RESULTS_SESSION_ID, sessionId);
		values.put(COLUMN_RESULTS_SCORE, score);
		values.put(COLUMN_RESULTS_RAW, raw);
		
		return mDatabase.update(TABLE_RESULTS, values, 
						COLUMN_RESULTS_RESULT_ID + "=" + resultId + " and " + 
						COLUMN_RESULTS_SESSION_ID + "=" + sessionId, 
				null) > -1;
	}
	
	public boolean insertSession(long sessionId, boolean isFinished, String configuration) {
		ContentValues values = new ContentValues();
		values.put(COLUMN_SESSIONS_SESSION_ID, sessionId);
		values.put(COLUMN_SESSIONS_IS_FINISHED, isFinished);
		values.put(COLUMN_SESSIONS_CONFIGURATION, configuration);
		
		return mDatabase.insert(TABLE_SESSIONS, null, values) > -1;
	}
	
	public boolean updateSession(long sessionId, boolean isFinished, String configuration) {
		ContentValues values = new ContentValues();
		values.put(COLUMN_SESSIONS_SESSION_ID, sessionId);
		values.put(COLUMN_SESSIONS_IS_FINISHED, isFinished);
		if(configuration != null)
			values.put(COLUMN_SESSIONS_CONFIGURATION, configuration);
		
		return mDatabase.update(TABLE_SESSIONS, values, 
						COLUMN_RESULTS_SESSION_ID + "=" + sessionId, 
				null) > -1;
	}
	
	public ResultGroupVector getBestResults() {
		ResultGroupVector vector = new ResultGroupVector();
	    
	    String query = "SELECT MAX(score), * FROM Results GROUP BY result_id;";
		
		Cursor cursor = mDatabase.rawQuery(query, null);
		
		cursor.moveToFirst();
	    while (!cursor.isAfterLast()) {
	    	ResultGroup r = resultFromCursor(cursor, 4);
	    	vector.add(r);
	    	cursor.moveToNext();
	    }
		
		cursor.close();
		
		return vector;
	}
	
	public ResultGroupVector getResultsForSession(long sessionId) {
		ResultGroupVector vector = new ResultGroupVector();
	    
	    String query = "SELECT * FROM Results WHERE session_id = " + sessionId + ";";
		
		Cursor cursor = mDatabase.rawQuery(query, null);
		
		cursor.moveToFirst();
	    while (!cursor.isAfterLast()) {
	    	ResultGroup r = resultFromCursor(cursor, 3);
	    	vector.add(r);
	    	cursor.moveToNext();
	    }
		
		cursor.close();
		
		return vector;
	}
	
	public SessionVector getSessions() {
		SessionVector vector = new SessionVector();
	    
	    String query = "SELECT * FROM Sessions ORDER BY session_id DESC;";
		
		Cursor cursor = mDatabase.rawQuery(query, null);
		
		cursor.moveToFirst();
	    while (!cursor.isAfterLast()) {
	    	Session r = sessionFromCursor(cursor);
	    	vector.add(r);
	    	cursor.moveToNext();
	    }
		
		cursor.close();
		
		return vector;
	}
	
	private ResultGroup resultFromCursor(Cursor cursor, int rawIndex) {
		ResultGroup resultGroup = new ResultGroup();
		
		String raw = cursor.getString(rawIndex); 
		resultGroup.fromJsonString(raw);
		
		return resultGroup;
	}
	
	private Session sessionFromCursor(Cursor cursor) {
		Session session = new Session();

		long sessionId = cursor.getLong(0);
		boolean isFinished = cursor.getInt(1) > 0;
		String configuration = cursor.getString(2);

		session.setSessionId(sessionId);
		session.setFinished(isFinished);
		session.setConfigurationName(configuration);
		
		return session;
	}

}
