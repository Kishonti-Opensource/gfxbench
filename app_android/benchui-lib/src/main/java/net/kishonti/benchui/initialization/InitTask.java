/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.util.HashMap;
import java.util.regex.Pattern;

import android.content.Context;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.method.ScrollingMovementMethod;
import android.text.util.Linkify;
import android.widget.ScrollView;
import android.widget.TextView;


public abstract class InitTask {
	protected Context mContext;
	protected String mName;
	protected int mRetryCount;
	
	public static final String TAG_PROGRESS = "progress";
	public static final String TAG_PROGRESS_MESSAGE = "message";
	
	private static final Progress mNoProgress = new Progress("", -1);
	
	public InitTask(Context context, String name, int retryCount) {
		mContext = context;
		mName = name;
		mRetryCount = retryCount;
	}

	abstract public Result run(HashMap<String, Object> params);
	
	public Progress getProgress() {
		return mNoProgress;
	}
	
	public String getName() {
		return mName;
	}
	
	public int getRetryCount() {
		return mRetryCount;
	}
	
	public void cancel() {
	}
	
	public UserInteraction getUserInteraction() {
		return null;
	}
	
	public UserInteraction getRetryUserInteraction() {
		return null;
	}
	
	public static class Progress {
		private String mStatus;
		private float mProgress;

		public Progress(String status, float prorgress) {
			mStatus = status;
			mProgress = prorgress;
		}

		public String getStatus() {
			return mStatus;
		}

		public double getProgress() {
			return mProgress;
		}
	}
	

	public static class Result {
		public boolean mSuccess;
		public String mMessage;
		
		public Result(boolean success, String message) {
			mSuccess = success;
			mMessage = message;
		}
	}
	
	public static class UserInteraction {
		private String mName;
		private String mBody;
		private String mPositive;
		private String mNegative;
		
		public UserInteraction(String name, String body, String positive, String negative) {
			mName = name;
			mBody = body;
			mPositive = positive;
			mNegative = negative;
		}
		
		public String getName() {
			return mName;
		}
		
		public SpannableString getBody() {
			final SpannableString s = new SpannableString(mBody);
			Linkify.addLinks(s, Linkify.ALL);
			return s;
		}
		
		public ScrollView getBodyTV(Context context) {
			final ScrollView scroller = new ScrollView(context);
			final TextView message = new TextView(context);
			message.setPadding(10, 10, 10, 10);
			message.setText(getBody());
			message.setMovementMethod(LinkMovementMethod.getInstance());
			scroller.addView(message);
			return scroller;
		}
		
		public String getPositive() {
			return mPositive;
		}
		
		public String getNegative() {
			return mNegative;
		}
	}
}
