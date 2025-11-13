/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.model;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.db.CompareSQLiteHelper;
import net.kishonti.benchui.db.ResultSQLiteHelper;
import net.kishonti.benchui.lists.resultlist.BestListDataProvider;
import net.kishonti.benchui.lists.resultlist.CompareListDataProvider;
import net.kishonti.benchui.lists.resultlist.DuelListDataProvider;
import net.kishonti.benchui.lists.resultlist.ResultListDataProvider;
import net.kishonti.benchui.lists.resultlist.ResultSelectionListDataProvider;
import net.kishonti.benchui.lists.testselect.TestListDataProvider;
import net.kishonti.swig.ApiDefinition;
import net.kishonti.swig.ApiDefinitionVector;
import net.kishonti.swig.CompareItem;
import net.kishonti.swig.CompareItemVector;
import net.kishonti.swig.CompareResult;
import net.kishonti.swig.CompareResultVector;
import net.kishonti.swig.Configuration;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.swig.ResultGroupVector;
import net.kishonti.swig.ResultItem;
import net.kishonti.swig.ResultItemVector;
import net.kishonti.swig.Session;
import net.kishonti.swig.SessionVector;
import net.kishonti.swig.StringVector;
import net.kishonti.swig.TestItem;
import net.kishonti.swig.TestItemVector;
import net.kishonti.swig.TestRepository;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

public class BenchmarkTestModel implements 
	TestListDataProvider,
	BestListDataProvider,
	ResultListDataProvider,
	ResultSelectionListDataProvider,
	CompareListDataProvider,
	DuelListDataProvider {

	private TestRepository repo;
	private CompareSQLiteHelper mCompareHelper;
	private ResultSQLiteHelper mResultHelper;
	private List<ResultsChangedListener> changeListeners = new ArrayList<ResultsChangedListener>();
	private Context mContext;
	
	private TestItemVector mTestVector;
	private TestItemVector mSelectedTestVector;
	private ResultItemVector mBestVector;
	private ResultItemVector mResultVector;
	private ResultItemVector mResultSelectorVector;
	private CompareResultVector mCompareVector;
	private CompareItemVector mDuelVector;
	
	private Boolean mIsInitialized = false;
	
	public String loginDependentText = "";
	
	public static final long BEST_SESSION_ID = Long.MAX_VALUE; 
	
	public BenchmarkTestModel(Context context, Boolean isMulticolumn) {
		mContext = context;
	}
	
	public void init(Context context, Boolean isMulticolumn) {
        String testlistString = readFromFile(context);
		repo = new TestRepository();
		try {
			repo.loadTestsFromJsonString(testlistString);
		} catch (ParseException e) {
			Log.e("BenchmarkModel", "TestFW Exception. Check benchmarktests.json", e);
		}

		setupConfigurations();

		loadCompare();
		
		File resultDbDir = mContext.getDir("db", Context.MODE_PRIVATE);
		mResultHelper = new ResultSQLiteHelper(mContext, resultDbDir.getAbsolutePath());
		mResultHelper.open();
		
		loadUserResults(context);
		
		mIsInitialized = true;
	}
	
	public Boolean isInitialized() {
		return mIsInitialized;
	}
	
	public void loadCompare() {
		if(mCompareHelper != null) mCompareHelper.close();
		String comparePath = ((BenchmarkApplication)mContext.getApplicationContext()).getCompareDatabasePath(((BenchmarkApplication)mContext.getApplicationContext()).getWorkingDir()); 
		mCompareHelper = new CompareSQLiteHelper(mContext, comparePath);
		mCompareHelper.open();
	}
	
	public void setupConfigurations() {
		if(repo == null) return;
			
		ApiDefinitionVector APIList = new ApiDefinitionVector();
		
		for (int i = 0; i < BenchmarkApplication.getApiDefinitions().size(); ++i) {
			ApiDefinition api = BenchmarkApplication.getApiDefinitions().get(i);
			APIList.add(api);
		}
        
        ApiDefinition gles31WithColorBufferFloat = new ApiDefinition();
        gles31WithColorBufferFloat.setType(ApiDefinition.Type.ES);
		gles31WithColorBufferFloat.setMajor(3);
		gles31WithColorBufferFloat.setMinor(1);
        StringVector gles31WithColorBufferFloatExtensions = new StringVector();
        gles31WithColorBufferFloatExtensions.add("GL_EXT_color_buffer_float");
        gles31WithColorBufferFloat.setExtensions(gles31WithColorBufferFloatExtensions);
        
        ApiDefinition gles31WithAEP = new ApiDefinition();
        gles31WithAEP.setType(ApiDefinition.Type.ES);
		gles31WithAEP.setMajor(3);
		gles31WithAEP.setMinor(1);
        StringVector gles31WithAEPExtensions = new StringVector();
        gles31WithAEPExtensions.add("GL_ANDROID_extension_pack_es31a");
        gles31WithAEP.setExtensions(gles31WithAEPExtensions);
        
        ApiDefinition gles32 = new ApiDefinition();
        gles32.setType(ApiDefinition.Type.ES);
		gles32.setMajor(3);
		gles32.setMinor(2);
		
		if(APIList.size() > 0) {
			Configuration conf = new Configuration();
			conf.setName("Basic android");
			conf.setType("GPU");
			StringVector features = new StringVector();
			features.add("battery");
            for (int i = 0; i < APIList.size(); ++i) {
                ApiDefinition api = APIList.get(i);
                if (api.isCompatibleWith(gles31WithColorBufferFloat) || api.isCompatibleWith(gles32)) {
                    features.add("float_extension");
                    break;
                }
            }
            for (int i = 0; i < APIList.size(); ++i) {
                ApiDefinition api = APIList.get(i);
                if (api.isCompatibleWith(gles31WithAEP) || api.isCompatibleWith(gles32)) {
                    features.add("AEP");
                    break;
                }
            }
			conf.setFeatures(features);
			conf.setApiDefinitions(APIList);
			repo.addConfiguration(conf); 
		}
		
		if(!Utils.isPoneOrTablet(mContext)) {
			try {
				repo.addTestIncompReason("gl_trex_battery", "PhoneOrTabletOnly");
			}
			catch(Exception ex) {
				Log.e("Model", "Error during disabling tests: " + ex.getLocalizedMessage());
			}
		}
	}
	
	public void loadUserResults(Context context) {
		if(repo != null) { 
			repo.setSessions(mResultHelper.getSessions());
			ResultGroupVector best = mResultHelper.getBestResults();
			repo.setBestResults(best);
			repo.setResultsForSession(best);
		}
	}
	
	public void loadResultsForSession(long sessionId, boolean groupedVariants) {
		if(repo != null) {
			repo.selectSession(sessionId);
			
			if(sessionId == BEST_SESSION_ID) {
				repo.setResultsForSession(mResultHelper.getBestResults());
			} else {
				repo.setResultsForSession(mResultHelper.getResultsForSession(sessionId));
			}
		}
	}
	
	public Session getSelectedSession() {
		if(repo != null)
			return repo.selectedSession();
		else
			return new Session();
	}
	
	public SessionVector getSessions() {
		if(repo != null)
			return repo.sessions();
		else
			return new SessionVector();
	}
	
	public void newSession(final long sessionId, final boolean cmd) {
		if(repo == null) return;
			
		new Handler(Looper.getMainLooper()).post(new Runnable() {
		    @Override
		    public void run() {
		    	mResultHelper.insertSession(sessionId, false, cmd ? "commandline" : repo.selectedConfiguration().name());

				loadUserResults(mContext);
				RefreshTestData();
		    }
		});
	}
	
	public void newResults(final long sessionId, final ResultGroup results) {
		if(repo == null) return;
		
		new Handler(Looper.getMainLooper()).post(new Runnable() {
		    @Override
		    public void run() {
				for(int i = 0; i < results.results().size(); i++) {
					Result result = results.results().get(i);
					mResultHelper.insertResult(result.resultId(), sessionId, result.score(), results.toJsonString());
				}

				loadUserResults(mContext);
				RefreshTestData();
		    }
		});
	}
	
	public void endSession(final long sessionId) {
		if(repo == null) return;
		
		new Handler(Looper.getMainLooper()).post(new Runnable() {
		    @Override
		    public void run() {
		    	mResultHelper.updateSession(sessionId, true, null);

				loadUserResults(mContext);
				RefreshTestData();
		    }
		});
	}
	
	private String readFromFile(Context context) {
		
	    String ret = "";
	
	    try {
	        InputStream inputStream = context.getAssets().open(
	        		BenchmarkApplication.getMinimalProps(context).appinfo_testjson);
	
	        if ( inputStream != null ) {
	            InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
	            BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
	            String receiveString = "";
	            StringBuilder stringBuilder = new StringBuilder();
	
	            while ( (receiveString = bufferedReader.readLine()) != null ) {
	                stringBuilder.append(receiveString).append("\n");
	            }
	
	            inputStream.close();
	            ret = stringBuilder.toString();
	        }
	    }
	    catch (FileNotFoundException e) {
	        Log.e("repository loading", "File not found: " + e.toString());
	    } catch (IOException e) {
	        Log.e("repository loading", "Can not read file: " + e.toString());
	    }
	
	    return ret;
	}

	
	public void clearAll(Context ctx) {
		mResultHelper.clearResults();
		
		loadUserResults(ctx);
		RefreshTestData();
	}
	
	public void registerResultChangedListener(ResultsChangedListener listener) {
		if(!changeListeners.contains(listener))
			changeListeners.add(listener);
	}
	
	public void unRegisterResultChangedListener(ResultsChangedListener listener) {
		if(changeListeners.contains(listener))
			changeListeners.remove(listener);
	}
	
	public void RefreshTestData() {
		for (ResultsChangedListener listener : changeListeners) {
			listener.HandleResultsChanged();
		}
	}
	
	
	/**
	 * ----------------------------------------------------------------------------------
	 * TestListDataProvider
	 * ----------------------------------------------------------------------------------
	 */

	@Override
	public TestItemVector getTestList() {
		mTestVector = new TestItemVector();
		if(repo != null) {
			mTestVector = TestRepository.headerRepeatedColumnLayout(repo.tests());
		}
		return mTestVector;
	}
	
	@Override
	public long getTestCount() {
		return mTestVector.size();
	}

	@Override
	public TestItem getTestForPosition(int position) {
		return mTestVector.get(position);
	}

	@Override
	public TestItemVector getSelectedTests(Boolean all) {
		mSelectedTestVector = new TestItemVector();
		if(repo != null) {
			if(all)
				mSelectedTestVector = repo.allAvailableTests();
			else
				mSelectedTestVector = repo.selectedAvailableTests();			
		}
		
		return mSelectedTestVector;
	}

	@Override
	public void saveTestSelection(String prefs) {
		if(repo == null) return;
		
		SharedPreferences sharedPref = mContext.getSharedPreferences(prefs, Context.MODE_PRIVATE);
		TestItemVector selected = repo.selectedAvailableTests();
		List<String> selectedTestIds = new ArrayList<String>();
		for (int i = 0; i < selected.size(); ++i) {
			selectedTestIds.add(selected.get(i).testId());
		}
		sharedPref.edit().putStringSet("Selection", new HashSet<String>(selectedTestIds)).commit();
	}

	@Override
	public void loadTestSelection(String prefs) {
		if(repo == null) return;
		
		SharedPreferences sharedPref = mContext.getSharedPreferences("test_selection", Context.MODE_PRIVATE);
		Set<String> loaded = sharedPref.getStringSet("Selection", null);
		if(loaded != null) {
			for (String testId : loaded) {
				setSelectionForTest(testId, true);
			}
		}
	}

	@Override
	public void addTestIncompReason(String testId, String reason) {
		if(repo == null) return;
		
		repo.addTestIncompReason(testId, reason);
	}
	
	@Override
	public void enableLoginDependentTests(boolean enable) {
		if(repo == null) return;
		
		TestItemVector tests = repo.tests();
		
		for(int i = 0; i < tests.size(); ++i) {
			TestItem test = tests.get(i);
			if(enable)
				repo.removeTestIncompReason(test.testId(), loginDependentText);
			else if(!loginDependentText.equals(""))
				repo.addTestIncompReason(test.testId(), loginDependentText);
		}
		
		RefreshTestData();
		RefreshTestData();
	}

	@Override
	public void setSelectionForGroup(String groupId, Boolean selected) {
		if(repo == null) return;
		
		repo.setGroupSelection(groupId, selected);
	}

	@Override
	public void setSelectionForTest(String testId, Boolean selected) {
		if(repo == null) return;
		
		repo.setTestSelection(testId, selected);
	}

	@Override
	public TestItem getTest(String testId) {
		if(repo != null)
			return repo.findTest(testId);
		else
			return new TestItem();
	}
	

	/**
	 * ----------------------------------------------------------------------------------
	 * BestListDataProvider
	 * ----------------------------------------------------------------------------------
	 */

	@Override
	public ResultItemVector getBestList() {
		mBestVector = new ResultItemVector();
		if(repo != null)
			mBestVector = TestRepository.headerRepeatedColumnLayout(repo.bestResults());
		return mBestVector;
	}

	@Override
	public ResultItem getBestResultForId(String resultId) {
		if(repo != null)
			return repo.findBestResult(resultId);
		else 
			return new ResultItem();
	}

	@Override
	public ResultItem getBestResultForPosition(int position) {
		return mBestVector.get(position);
	}

	@Override
	public long getBestCount() {
		return mBestVector.size();
	}


	/**
	 * ----------------------------------------------------------------------------------
	 * ResultListDataProvider
	 * ----------------------------------------------------------------------------------
	 */

	@Override
	public ResultItemVector getResultList() {
		mResultVector = new ResultItemVector();
		if(repo != null)
			mResultVector = TestRepository.filterNotAvailableRows(
				TestRepository.headerRepeatedColumnLayout(repo.resultsForSession()));
		return mResultVector;
	}

	@Override
	public ResultItem getResultForPosition(int position) {
		return mResultVector.get(position);
	}

	@Override
	public long getResultCount() {
		return mResultVector.size();
	}
	
	

	/**
	 * ----------------------------------------------------------------------------------
	 * ResultSelectorListDataProvider
	 * ----------------------------------------------------------------------------------
	 */
	
	@Override
	public ResultItemVector getResultSelectorList(boolean isHeadered) {
		mResultSelectorVector = new ResultItemVector();
		if(repo != null) {
			if(isHeadered) {
				mResultSelectorVector = TestRepository.headerRepeatedColumnLayout(repo.bestResults());
			} else {
				mResultSelectorVector = TestRepository.simpleColumnLayout(repo.bestResults());
			}
		}
		return mResultSelectorVector;
	}

	@Override
	public int getResultSelectorItemPosition(String resultId) {
		for(int i = 0; i < mResultSelectorVector.size(); ++i) {
			if(mResultSelectorVector.get(i).resultId().equals(resultId))
				return i;
		}
		
		return 0;
	}

	@Override
	public ResultItem getResultSelectorItemForPosition(int position) {
		return mResultSelectorVector.get(position);
	}

	@Override
	public long getResultSelectorCount() {
		return mResultSelectorVector.size();
	}
	
	
	/**
	 * ----------------------------------------------------------------------------------
	 * CompareListDataProvider
	 * ----------------------------------------------------------------------------------
	 */
	@Override
	public CompareResultVector getCompareList(String result_id, String filter, CompareSQLiteHelper.FormFactorFilter formFilter) {
		if(repo == null) return mCompareVector = new CompareResultVector();
		
		ResultItem result = repo.findBestResult(result_id);
		CompareResultVector compareResults = mCompareHelper.getCompareListFor(
				result.baseId(), 
				result.variantPostfix().equals("") ? "on" : result.variantPostfix().replace("_", ""), 
				result.resultPostfix().replace("_", ""), 
				formFilter, 
				filter);
		repo.setCompareResults(result_id, compareResults);
		mCompareVector = repo.compareResults();
		return mCompareVector;
	}

	@Override
	public CompareResult getCompareItemForPosition(int position) {
		return mCompareVector.get(position);
	}

	@Override
	public long getCompareListCount() {
		return mCompareVector.size();
	}
	
	

	/**
	 * ----------------------------------------------------------------------------------
	 * DuelListDataProvider
	 * ----------------------------------------------------------------------------------
	 */
	
	public CompareResultVector getDeviceResults(String device) {
		if(repo == null) return new CompareResultVector();
		
		CompareResultVector deviceVector = mCompareHelper.getResultsForDevice(device);
		repo.setDuelResults(deviceVector);
		return deviceVector;
	}

	@Override
	public CompareItemVector getDuelList() {
		mDuelVector = new CompareItemVector();
		if(repo != null)
			mDuelVector = repo.duelResults();
		return mDuelVector;
	}

	@Override
	public int getDuelItemPosition(String resultId) {
		for(int i = 0; i < mDuelVector.size(); ++i) {
			if(mDuelVector.get(i).resultId().equals(resultId))
				return i;
		}
		
		return 0;
	}

	@Override
	public CompareItem getDuelItemForPosition(int position) {
		return mDuelVector.get(position);
	}

	@Override
	public long getDuelCount() {
		return mDuelVector.size();
	}
}
