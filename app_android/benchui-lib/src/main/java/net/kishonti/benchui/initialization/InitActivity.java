/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.io.InvalidObjectException;
import java.util.HashMap;
import java.util.List;

import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.initialization.InitTask.Result;
import net.kishonti.customcomponents.CustomProgressbar;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.WindowManager;
import android.widget.TextView;

public class InitActivity extends ManagedLifeActivity {
	public final static int CODE_FAIL = 0;
	public final static int CODE_SUCCESS = 1;
	protected static final String LOG_INIT = "Initialization";
	protected static final long PROGRESS_UPDATE_DELAY_MS = 200;
	
	//private static final int PROGRESS_MAX = 10000;
	
	private int mLoopCounter = 0;
	private String mLoopedTask = "";

	private TextView mTextView;
	private CustomProgressbar mProgressBar;
	private int mRetryTime = 1000;
	private Handler mHandler;
	private Runnable mRetry;
	private Runnable mSubtaskProgress;
	private InitTaskRunner mTaskRunner;
	private InitTask[] mInitTasks;
	private AlertDialog mCurrentDialog;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
//		setContentView(R.layout.activity_init);
		setContentView(Utils.getLayoutId(this, "activity_init"));
		
		List<InitTask> tasklist = ((InitializerApplication) getApplication()).getInitTasks();
		mInitTasks = new InitTask[tasklist.size()];
		tasklist.toArray(mInitTasks);

		mTextView = (TextView) findViewById(R.id.init_statusTxt);
		mTextView.setText("Initializing network manager.");

		mProgressBar = (CustomProgressbar) findViewById(R.id.init_progressbar);
		mProgressBar.onProgressChanged(0, false);
		
		if(mProgressBar.getTag() != null && mProgressBar.getTag().toString().equals("xlarge")) {
			setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	    } else {
	    	setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
	    }

		mHandler = new Handler();
		mRetry = new Runnable() {
			@Override
			public void run() {
				init();
			}
		};
		
		mSubtaskProgress = new Runnable() {
			@Override
			public void run() {
				if(mTaskRunner != null) {
					try {
						InitTask.Progress p = mTaskRunner.getProgress();
						displayProgress(p, p.getProgress() <= 0);
					} catch (InvalidObjectException e) {
						e.printStackTrace();
						Log.e(LOG_INIT, "No task to get progress from.", e);
					}
					mHandler.postDelayed(this, PROGRESS_UPDATE_DELAY_MS);
				}
			}
		};
	}

	private void init() {
		Log.i("Init", "Initialization attempt.");
		mTaskRunner = new InitTaskRunner();
		mTaskRunner.execute(mInitTasks);
	}

	private void initSuccess(boolean success, HashMap<String, Object> params) {
		((InitializerApplication) getApplication()).initEnd(success, params);
		finish();
	}
	
	private void displayProgress(InitTask.Progress progress, boolean infinite) {
		if(progress.getProgress() >= 0) {
			//mProgressBar.setProgress((int) (progress.getProgress() * PROGRESS_MAX));
			mProgressBar.onProgressChanged(progress.getProgress(), !infinite);
			mTextView.setText(progress.getStatus());
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		init();
	}

	@Override
	protected void onPause() {
		super.onPause();
		getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		mProgressBar.onProgressChanged(0, true);
		if(mTaskRunner != null) {
			mTaskRunner.mCurrentUserInteraction = null;
			mTaskRunner.cancelCurrentTask();
			mTaskRunner.cancel(false);
			mTaskRunner = null;
		}
		if(mCurrentDialog != null) {
			mCurrentDialog.dismiss();
		}
		mHandler.removeCallbacks(mRetry);
		mHandler.removeCallbacks(mSubtaskProgress);
	}

	@Override
	public void onBackPressed() {
		Intent i = new Intent();
		i.setAction(Intent.ACTION_MAIN);
		i.addCategory(Intent.CATEGORY_HOME);
		this.startActivity(i);
	}
	
	public class InitTaskRunner extends AsyncTask<InitTask, InitTask.Progress, InitTask.Result> {
		
		private final Object mLock = new Object();
		private InitTask mCurrentTask;
		private InitTask.Result mCurrentResult;
		private HashMap<String, Object> mParams = new HashMap<String, Object>();
		private boolean mUserConfirmed = true;
		private InitTask.UserInteraction mCurrentUserInteraction = null;
		
		private Runnable mShouldContinue = new Runnable() {
			@Override
			public void run() {
				if(mCurrentUserInteraction == null) {
					synchronized (mShouldContinue) {
						mShouldContinue.notify();
						mCurrentDialog = null;
						return;
					}
				}
				if(mCurrentUserInteraction.getNegative() != null && mCurrentUserInteraction.getPositive() != null) {
					mCurrentDialog = new AlertDialog.Builder(InitActivity.this)
					.setCancelable(false)
					.setPositiveButton(mCurrentUserInteraction.getPositive(), new DialogInterface.OnClickListener() {

						@Override
						public void onClick(DialogInterface dialog, int which) {
							mUserConfirmed = true;
						}
					})
					.setNegativeButton(mCurrentUserInteraction.getNegative(), new DialogInterface.OnClickListener() {

						@Override
						public void onClick(DialogInterface dialog, int which) {
							mUserConfirmed = false;
						}
					})
					.setTitle(mCurrentUserInteraction.getName())
					.setView(mCurrentUserInteraction.getBodyTV(InitActivity.this))
					.create();
				} else if(mCurrentUserInteraction.getNegative() != null) {
					mCurrentDialog = new AlertDialog.Builder(InitActivity.this)
					.setCancelable(false)
					.setNegativeButton(mCurrentUserInteraction.getNegative(), new DialogInterface.OnClickListener() {

						@Override
						public void onClick(DialogInterface dialog, int which) {
							mUserConfirmed = false;
						}
					})
					.setTitle(mCurrentUserInteraction.getName())
					.setView(mCurrentUserInteraction.getBodyTV(InitActivity.this))
					.create();
				} else {
					mCurrentDialog = new AlertDialog.Builder(InitActivity.this)
					.setCancelable(false)
					.setPositiveButton(mCurrentUserInteraction.getPositive(), new DialogInterface.OnClickListener() {

						@Override
						public void onClick(DialogInterface dialog, int which) {
							mUserConfirmed = true;
						}
					})
					.setTitle(mCurrentUserInteraction.getName())
					.setView(mCurrentUserInteraction.getBodyTV(InitActivity.this))
					.create();
				}
				mCurrentDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {

					@Override
					public void onDismiss(DialogInterface dialog) {
						synchronized (mShouldContinue) {
							mShouldContinue.notify();
							mCurrentDialog = null;
						}
					}
				});
				mCurrentDialog.show();
			}
		};
		
		private boolean isActive() {
			return this == mTaskRunner;
		}
		
		public void cancelCurrentTask() {
			synchronized (mLock) {
				if(mCurrentTask != null) {
					mCurrentTask.cancel();
				}
			}
		}
		
		public InitTask.Progress getProgress() throws InvalidObjectException {
			synchronized (mLock) {
				if(mCurrentTask != null) {
					return mCurrentTask.getProgress();
				} else {
					throw new InvalidObjectException("No current task.");
				}
			}
		}
		
		public InitTask.Result getResult() {
			synchronized (mLock) {
				return mCurrentResult;
			}
		}
		
		private void setResult(InitTask.Result result) {
			synchronized (mLock) {
				mCurrentResult = result;
			}
		}

		@Override
		protected Result doInBackground(InitTask... tasks) {
			
			synchronized (mLock) {
				if(isActive()) mCurrentResult = new Result(true, "Nothing happened.");
			}
			for (int i = 0; i < tasks.length && isActive() && mUserConfirmed; i++) {
				synchronized (mLock) {
					if(isActive()) mCurrentTask = tasks[i];
				}
				float overallProgress = i/(float)tasks.length;
				publishProgress(new InitTask.Progress(mCurrentTask.getName(), overallProgress));
				mHandler.post(mSubtaskProgress);
				InitTask.Result result = mCurrentTask.run(mParams);
				mHandler.removeCallbacks(mSubtaskProgress);
				
				// simulate long running task
//				try { Thread.sleep(2000); } catch (InterruptedException e) { }

				// user notification
				mCurrentUserInteraction = mCurrentTask.getUserInteraction();
				if(mCurrentUserInteraction != null) {
					runOnUiThread(mShouldContinue);
					try {
						synchronized (mShouldContinue) {
							mShouldContinue.wait();
						}
					} catch (InterruptedException e1) {
						e1.printStackTrace();
						mUserConfirmed = false;
					}
				}
				mCurrentUserInteraction = null;
				
				setResult(result);
				if(!result.mSuccess) {
					
					// Exit dialog if there was too many retry with this task
					if(mLoopedTask.equals(mCurrentTask.mName) || mLoopedTask.isEmpty()) {
						mLoopedTask = mCurrentTask.mName;
						mLoopCounter++;
					}
					if(mLoopCounter >= mCurrentTask.getRetryCount()) {
						mCurrentUserInteraction = mCurrentTask.getRetryUserInteraction();
						if(mCurrentUserInteraction != null) {
							runOnUiThread(mShouldContinue);
							try {
								synchronized (mShouldContinue) {
									mShouldContinue.wait();
								}
							} catch (InterruptedException e1) {
								e1.printStackTrace();
								mUserConfirmed = false;
							}
							
							if(!mUserConfirmed) {
								initSuccess(false, mParams);
								break;
							} else {
								mLoopCounter = 0;
							}
						}
					}
					
					publishProgress(new InitTask.Progress(result.mMessage, overallProgress));
					break;
				} else {
					// remove empty named tasks
					if(!mCurrentTask.mName.isEmpty() && mCurrentTask.mName.equals(mLoopedTask)) {
						mLoopedTask = "";
						mLoopCounter = 0;
					}
				}
				
				
				if(!mUserConfirmed) {
					initSuccess(false, mParams);
					break;
				}
			}
			return getResult();
		}
		
		protected void onProgressUpdate(InitTask.Progress... progress) {
			if(isActive()) {
				displayProgress(progress[0], true);
			}
		}

		protected void onPostExecute(InitTask.Result result) {
			if(isActive()) {
				mHandler.removeCallbacks(mSubtaskProgress);
				if (!result.mSuccess) {
					mHandler.postDelayed(mRetry, mRetryTime);
				} else {
					initSuccess(true, mParams);
				}
				mCurrentTask = null;
			}
		}
	}
}
