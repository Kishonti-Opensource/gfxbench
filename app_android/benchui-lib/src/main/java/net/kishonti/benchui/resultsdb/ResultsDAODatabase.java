/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.resultsdb;

import java.util.List;

import de.greenrobot.dao.query.LazyList;
import de.greenrobot.dao.query.WhereCondition;
import gfxc.dao.DaoMaster;
import gfxc.dao.DaoSession;
import gfxc.dao.DaoMaster.DevOpenHelper;
import gfxc.dao.Device;
import gfxc.dao.DeviceDao;
import gfxc.dao.Result;
import gfxc.dao.ResultDao;
import gfxc.dao.Test;
import gfxc.dao.TestDao;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

public class ResultsDAODatabase {
	
	private static SQLiteDatabase db;
	
	private static DaoMaster daoMaster;
    private static DaoSession daoSession;
	private static TestDao testDao;
	private static ResultDao resultDao;
	private static DeviceDao deviceDao;
	
	protected static void createDAO() {
        daoMaster = new DaoMaster(db);
        daoSession = daoMaster.newSession();
        testDao = daoSession.getTestDao();
        resultDao = daoSession.getResultDao();
        deviceDao = daoSession.getDeviceDao();
	
	}
	
	public static SQLiteDatabase openDAODB(Context con,String daoname) {
        DevOpenHelper helper = new DaoMaster.DevOpenHelper(con, daoname, null);
        db = helper.getWritableDatabase();
        createDAO();
		return db;
	}
	
	public static SQLiteDatabase createDAODB(Context con,String dbname) {
        DevOpenHelper helper = new DaoMaster.DevOpenHelper(con, dbname, null);
        db = helper.getWritableDatabase();
        createDAO();
		return db;
	}
	
	public static SQLiteDatabase openSQLDB(String path) {
		if (db != null) {
			db.close();
			db = null;
		}
		try {
			db = SQLiteDatabase.openDatabase(path, null, SQLiteDatabase.OPEN_READONLY | SQLiteDatabase.NO_LOCALIZED_COLLATORS);
			
			if(db!=null) createDAO();

		} catch (Exception e) {
			Log.d("RESULTSDATABASE", "Could not open Results database. Not required.");
		}
		return db;
	}
	
	public static LazyList<Result> getResultsForTest(String test_id, int subresult_id, String device_filter, boolean displaydesktop) {
		if (db == null) return null;
		
		WhereCondition cond = null;
		if(!displaydesktop) {
			if(!device_filter.isEmpty()) {
				cond = new WhereCondition.StringCondition("DEVICE_ID IN" + "(SELECT DEVICE._id FROM DEVICE WHERE DEVICE.NAME LIKE '%"+device_filter+"%' AND DEVICE.FORM_FACTOR LIKE '%mobile%' )");
			} else {
				cond = new WhereCondition.StringCondition("DEVICE_ID IN" + "(SELECT DEVICE._id FROM DEVICE WHERE DEVICE.FORM_FACTOR LIKE '%mobile%')");
			}
		} else {
			cond = new WhereCondition.StringCondition("DEVICE_ID IN" + "(SELECT DEVICE._id FROM DEVICE WHERE DEVICE.NAME LIKE '%"+device_filter+"%')");
		}
		
//		if(!device_filter.isEmpty())
			return resultDao.queryBuilder()
					.where(new WhereCondition.StringCondition("TEST_ID IN" + "(SELECT TEST._id FROM TEST WHERE TEST.STRING_ID = '"+test_id+"' AND TEST.SUBRESULT_ID="+subresult_id+")"),cond)
					.orderDesc(ResultDao.Properties.Score)
					.listLazyUncached();
//		else
//			return resultDao.queryBuilder()
//					.where(new WhereCondition.StringCondition("TEST_ID IN" + "(SELECT TEST._id FROM TEST WHERE TEST.STRING_ID = '"+test_id+"' AND TEST.SUBRESULT_ID="+subresult_id+")"))
//					.orderDesc(ResultDao.Properties.Score)
//					.listLazyUncached();
	}
	
	public static List<Result> getResultsForDevice(long device_id) {
		if (db == null) return null;
		
		Device d = deviceDao.queryBuilder().where(DeviceDao.Properties.Id.eq(device_id)).unique();
		
		if(d==null)
			return null;
		
		return d.getResults();
	}

	public static String getTestUnit(String test_id, int subresult_id) {
		if (db == null) return null;
		
		Test t = testDao.queryBuilder().where(TestDao.Properties.StringId.eq(test_id),TestDao.Properties.SubresultId.eq(Long.valueOf(subresult_id))).unique();
		
		if(t==null)
			return null;
		
		return t.getMetric();
	}
}
