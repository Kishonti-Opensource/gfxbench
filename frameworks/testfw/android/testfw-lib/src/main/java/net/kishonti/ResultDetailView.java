/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti;

import java.io.File;
import java.io.FileOutputStream;

//import net.kishonti.theme.Localizator;
//import net.kishonti.testselect_lib.resultlist.ResultFormatter;
//import net.kishonti.testselect_lib.resultlist.ResultFormatter.FormattedResult;

//import org.json.JSONArray;
//import org.json.JSONException;
//import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class ResultDetailView extends WebView {

	static final private String TAG = "ResultWebView";
	private String mResultJsonString;
    File mFile = null;
    ResultDetailDataProvider mDataProvider = null;
    WebViewClient mClient = null;
    
    public static class ResultDetailData {
    	public String resultJsonString;
    	public String resultName;
    	public String resultFlags;
    	
    	public ResultDetailData(String json, String name, String flags) {
    		resultJsonString = json;
    		resultName = name;
    		resultFlags = flags;
    	}
    }
    
    public interface ResultDetailDataProvider {
    	public ResultDetailData getData();
    }
	
	public ResultDetailView(Context context) {
		super(context);
		setup();
	}
    
    public ResultDetailView(Context context, AttributeSet attrs) {
		super(context, attrs);
		setup();
	}

	public ResultDetailView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		setup();
	}

    @SuppressLint("SetJavaScriptEnabled") 
    private void setup() {
    	String manuf = android.os.Build.MANUFACTURER;
    	String brand = android.os.Build.BRAND;
    	
    	if(!manuf.equals("Amazon") && !brand.equals("Amazon")) {
    		setLayerType(View.LAYER_TYPE_SOFTWARE, null);
    	}
    	
		WebSettings webSettings = getSettings();
		webSettings.setJavaScriptEnabled(true);
		webSettings.setAllowFileAccessFromFileURLs(true);
    }
    
    public void presentData() {
    	
    	if(mDataProvider != null) {
    		final ResultDetailData data = mDataProvider.getData();
    		
    		if(data.resultName == null) return;
    		if(data.resultJsonString == null) return;
    		if(data.resultFlags == null) return;
    		
    		mResultJsonString = data.resultJsonString;
    		Context context = getContext();
    		
    		String filename = "diagram_result.json";
    		FileOutputStream outputStream;

    		mFile = new File(context.getFilesDir(), filename);
    		try {
    		  outputStream = context.openFileOutput(filename, Context.MODE_PRIVATE);
    		  outputStream.write(mResultJsonString.getBytes());
    		  outputStream.close();
    		} catch (Exception e) {
    		  e.printStackTrace();
    		}
    		
    		mClient = new WebViewClient() {  
    			@Override  
    			public boolean shouldOverrideUrlLoading(WebView view, String url)  
    			{  
    			  view.loadUrl(url);
    			  return true;
    			}
    			@Override
                public void onPageFinished(WebView view, String url) {
                    Log.d(TAG, "onPageFinished with url " + url);
                    
                    // Orientation change can null our view...
                    // We should del this on View del....
                    if(view != null && mFile != null && data != null) {
                    	view.loadUrl("javascript:updateResult('" + mFile.toURI().toString().replace("file:/", "file:///") + "', '" + data.resultName + "', " + !data.resultFlags.isEmpty() + ", '" + data.resultFlags + "')");
                    }
                }
    		};
    		
    		setWebViewClient(mClient);
    		loadUrl("file:///android_asset/resultdetail/result_ng.html");
    	}
    }
    
    public void setDataProvider(ResultDetailDataProvider provider) {
    	mDataProvider = provider;
    }
    
    @Override
    public void onPause() {
    	stopLoading();
    	setWebViewClient(null);
    	mClient = null;
    	super.onPause();
    }
    
    
	
	public void cleanup() {
    	if(mFile != null)
    		mFile.delete();
    	mFile = null;
	}
}
