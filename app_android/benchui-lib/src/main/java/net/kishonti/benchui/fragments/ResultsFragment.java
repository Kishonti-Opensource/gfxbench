/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import java.text.DateFormat;
import java.util.Date;
import android.graphics.Point;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.view.View.OnLayoutChangeListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Animation.AnimationListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;

import net.kishonti.benchui.BatteryActivity;
import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.fragments.HomeFragment.HomeState;
import net.kishonti.benchui.lists.adapters.TestRunLazyListAdapter;
import net.kishonti.benchui.lists.resultlist.BestListAdapter;
import net.kishonti.benchui.lists.resultlist.ResultListAdapter;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.benchui.model.ResultsChangedListener;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultItem;
import net.kishonti.swig.Session;
import net.kishonti.swig.SessionVector;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListView;

public class ResultsFragment extends BaseFragment implements 
	OnClickListener, OnItemClickListener, OnItemSelectedListener, OnItemLongClickListener, 
	AnimationListener, ResultsChangedListener {
	
	/**
	 * Root views and backgrounds.
	 */
	private View mRootView;
	private View mInfoBack;
	
	/**
	 * Navigation
	 */
	private Button mNavbarLeft;
	private Button mNavbarRight;
	private TextView mNavbarTitle;
	
	private Button mExtraNavbarLeft;
	private Button mExtraNavbarRight;
	private TextView mExtraNavbarTitle;
	private View mExtraNavbarBack;
	
	/**
	 * Main controls
	 */
	private ThemeListView mTestList;
	private ListView mHistoryList;
	private ImageView mInfoImage;
	private TextView mInfoText;
	private String mInfoTitle = "Information";
	private ResultsState mInfoFromState = ResultsState.BEST;

	private TestRunLazyListAdapter mHistoryAdapter;
	private SessionVector mSessionList;
	
	private ResultListAdapter mResultAdapter;
	private BestListAdapter mBestAdapter;
	
	private Session mBestSession;
	
	/**
	 * State management
	 */
	
	public enum ResultsState {
		BEST, HISTORY, RESULT, INFO
	}
	private ResultsState mState = ResultsState.BEST;
	private Animation mAnim_PageLeftIn;
	private Animation mAnim_PageLeftOut;
	private Animation mAnim_PageRightIn;
	private Animation mAnim_PageRightOut;
	private AnimationState mAnimState;

	private String mSelectedResultTitle = "";

	private int mRootWidth = 0;
	private float mThirdWidth = 0;
	private float mTwoThirdWidth = 0;
	
	/**
	 * Adapters
	 */
	private long mSelectedSessionId = -1;
	
	public void refreshModel() {
		BenchmarkApplication.getModel(getActivity()).registerResultChangedListener(this);
		
		String params = BenchmarkApplication.getAppModel().getFragmentParams(this);
		
		if(Utils.isTablet(getActivity())) {
			if(mBestSession == null) {
				mBestSession = new Session();
				mBestSession.setConfigurationName("");
				mBestSession.setFinished(true);
				mBestSession.setSessionId(BenchmarkTestModel.BEST_SESSION_ID);
			}
			mSessionList = new SessionVector();
			mSessionList.add(mBestSession);
			
			SessionVector sv = BenchmarkApplication.getModel(getActivity()).getSessions();
			for(int i = 0; i < sv.size(); i++) {
				mSessionList.add(sv.get(i));
			}
			
		} else {
			mSessionList = BenchmarkApplication.getModel(getActivity()).getSessions();
		}
		
		mHistoryAdapter = new TestRunLazyListAdapter(getActivity(), mSessionList);
		mHistoryList.setAdapter(mHistoryAdapter);
		mHistoryAdapter.notifyDataSetChanged();
		
		if(!params.equals("")) {
			mSelectedSessionId = Long.parseLong(params);
			
		}
		if(mSelectedSessionId <= 0) {
			mSelectedSessionId = BenchmarkTestModel.BEST_SESSION_ID;
		}
		
		if(mState == ResultsState.RESULT && mSelectedSessionId > 0) {
			loadSession(mSelectedSessionId);
		} else {
			loadSession(BenchmarkTestModel.BEST_SESSION_ID);
		}
	}
	
	public void loadSession(long session_id) {
		BenchmarkTestModel model = BenchmarkApplication.getModel(getActivity()); 
		mSelectedSessionId = session_id; 
		
		if(session_id == BenchmarkTestModel.BEST_SESSION_ID) {
			model.getBestList();
			mTestList.setAdapter(mBestAdapter);
			mBestAdapter.notifyDataSetChanged();
			
		} else {
			model.loadResultsForSession(session_id, Utils.isTablet(getActivity()));
			model.getResultList();
			mTestList.setAdapter(mResultAdapter);
			if(mResultAdapter != null)
				mResultAdapter.notifyDataSetChanged();
		}
	}

	@Override
	public void HandleResultsChanged() {
		mState = ResultsState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
		setupState();
		setupText();
		refreshModel();
	}
	
	/**
	 * Lifetime functions
	 */
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		mRootView = inflater.inflate(Utils.getLayoutId(getActivity(), "fragment_results"), container, false);

		if(BenchmarkApplication.getAppModel().getFragmentState(this).equals("")) {
			BenchmarkApplication.getAppModel().SetFragmentState(this, mState.name());
		}
        
		mInfoBack = (View)mRootView.findViewById(R.id.results_infoBack);
        
		mTestList = (ThemeListView)mRootView.findViewById(R.id.results_testList);
		mHistoryList = (ListView)mRootView.findViewById(R.id.results_historyList);
		mInfoImage = (ImageView)mRootView.findViewById(R.id.results_infoImage);
		mInfoText = (TextView)mRootView.findViewById(R.id.results_infoText);
		
		mNavbarLeft = (Button)mRootView.findViewById(R.id.headerButtonLeft);
		mNavbarRight = (Button)mRootView.findViewById(R.id.headerButtonRight);
		mNavbarTitle = (TextView)mRootView.findViewById(R.id.headerTitle);
//		mNavbarIcon = (ImageView)mRootView.findViewById(R.id.headerIcon);
		
		// layout width needs setup here if tablet. (not needed if animation is not present. use linearlayout in that case)
		if(Utils.isTablet(getActivity())) {
		    mRootView.addOnLayoutChangeListener(new OnLayoutChangeListener() {
				@Override
				public void onLayoutChange(View v, int left, int top, int right,
						int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
					
					Point actualSize = new Point(right - left, bottom - top);
					Point oldSize = new Point(oldRight - oldLeft, oldBottom - oldTop);
					
					if(actualSize.x != oldSize.x || actualSize.y != oldSize.y)
					{
						Point size = actualSize;
						mRootWidth = size.x;
						mThirdWidth = (int)(mRootWidth * (1.0f/3.0f));
						mTwoThirdWidth = (int)(mRootWidth - mThirdWidth);
						
						LinearLayout.LayoutParams historyParams = (LinearLayout.LayoutParams) mHistoryList.getLayoutParams();
						historyParams.width = (int)mThirdWidth;
						mHistoryList.setLayoutParams(historyParams);
						
						LinearLayout.LayoutParams listParams = (LinearLayout.LayoutParams) mTestList.getLayoutParams();
						listParams.width = (int)mTwoThirdWidth;
						mTestList.setLayoutParams(listParams);

						LinearLayout.LayoutParams infoParams = (LinearLayout.LayoutParams) mInfoBack.getLayoutParams();
						infoParams.width = (int)mThirdWidth;
						mInfoBack.setLayoutParams(infoParams);
					}
				}
			});
		}
		
		if(Utils.isTablet(getActivity())) {
			mExtraNavbarBack = mRootView.findViewById(R.id.results_navbar_extraBack);
			mExtraNavbarLeft = (Button)mRootView.findViewById(R.id.results_navbar_extraLeftButton);
			mExtraNavbarRight = (Button)mRootView.findViewById(R.id.results_navbar_extraRightButton);
			mExtraNavbarTitle = (TextView)mRootView.findViewById(R.id.results_navbar_extraTitle);
		}
		
		// Setup the button handlers
		mNavbarLeft.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		mHistoryList.setOnItemClickListener(this);
		if(Utils.isTablet(getActivity())) {
			mExtraNavbarLeft.setOnClickListener(this);
			mExtraNavbarRight.setOnClickListener(this);
		}

	    mTestList.setDescendantFocusability(ViewGroup.FOCUS_AFTER_DESCENDANTS);
	    mTestList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
	    mTestList.setOnItemLongClickListener(this); 
	    mTestList.setOnItemSelectedListener(this);
		mTestList.setOnItemClickListener(this);

	    mAnim_PageRightIn = AnimationUtils.loadAnimation(getActivity(), R.anim.result_right_in);
	    mAnim_PageRightOut = AnimationUtils.loadAnimation(getActivity(), R.anim.result_right_out);
	    mAnim_PageLeftIn = AnimationUtils.loadAnimation(getActivity(), R.anim.result_left_in);
	    mAnim_PageLeftOut = AnimationUtils.loadAnimation(getActivity(), R.anim.result_left_out);
	    
	    mAnim_PageRightIn.setAnimationListener(this);
	    mAnim_PageRightOut.setAnimationListener(this);
	    mAnim_PageLeftIn.setAnimationListener(this);
	    mAnim_PageLeftOut.setAnimationListener(this);
		
		return mRootView;
	}

	@Override
	public void onResume() {
		super.onResume();
		
		mState = ResultsState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));

		mResultAdapter = new ResultListAdapter(getActivity(), BenchmarkApplication.getModel(getActivity()));
		mBestAdapter = new BestListAdapter(getActivity(), BenchmarkApplication.getModel(getActivity()));

		setupState();
		setupText();
		
		if(((BenchmarkApplication)getActivity().getApplication()).mIsInitialized)
			refreshModel();
	}
	
	@Override
	public void onPause() {
		super.onPause();
		
		BenchmarkApplication.getModel(getActivity()).unRegisterResultChangedListener(this);
		mBestSession = null;
		mSessionList = null;
		
		mHistoryAdapter = null;
		mHistoryList.setAdapter(null);
	}
	
	/**
	 * Manages the state change visibility.
	 */
	private void setupState() {
		if(Utils.isTablet(getActivity())) {
			if(mState == ResultsState.INFO) {
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.GONE);
				mInfoBack.setVisibility(View.VISIBLE);
				mExtraNavbarBack.setVisibility(View.VISIBLE);
				mNavbarLeft.setVisibility(View.GONE);
				mNavbarRight.setVisibility(View.INVISIBLE);
				
			} else {
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.GONE);
				mExtraNavbarBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.GONE);
				mNavbarRight.setVisibility(View.INVISIBLE);
			}
			
		} else {
			if(mState == ResultsState.BEST) {
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.GONE);
				mInfoBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.GONE);
				
			} else if(mState == ResultsState.RESULT) {
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.GONE);
				mInfoBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.VISIBLE);
				
			} else if(mState == ResultsState.HISTORY) {
				mTestList.setVisibility(View.GONE);
				mHistoryList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.VISIBLE);
				
			} else if(mState == ResultsState.INFO) {
				mTestList.setVisibility(View.GONE);
				mHistoryList.setVisibility(View.GONE);
				mInfoBack.setVisibility(View.VISIBLE);
				mNavbarLeft.setVisibility(View.VISIBLE);
				
			}
		}
	}
	
	/**
	 * Setup state dependent texts
	 */
	private void setupText() {
		if(mState == ResultsState.BEST) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "BestResults"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), Utils.isTablet(getActivity()) ? "" : "More"));
			if(Utils.isTablet(getActivity())) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			}
			
		} else if(mState == ResultsState.RESULT) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), Utils.isTablet(getActivity()) ? "" : "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), mSelectedResultTitle));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			if(Utils.isTablet(getActivity())) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			}
			
		} else if(mState == ResultsState.HISTORY) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), Utils.isTablet(getActivity()) ? "" : "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "ResultHistory"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			if(Utils.isTablet(getActivity())) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			}
			
		} else if(mState == ResultsState.INFO) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), Utils.isTablet(getActivity()) ? "" : "Back"));
			mNavbarTitle.setText(Utils.isTablet(getActivity()) ? mNavbarTitle.getText().toString() : Localizator.getString(this.getActivity(), mInfoTitle));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			if(Utils.isTablet(getActivity())) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), mInfoTitle));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), "Close"));
			}
			
		}
	}
	
	
	/**
	 * --------------------------------------------------------------------------------------------------------------
	 * Button handling
	 * --------------------------------------------------------------------------------------------------------------
	 */
	
	/**
	 * Handler for the normal buttons
	 */
	@Override
	public void onClick(View v) {
		if(v.getId() == R.id.headerButtonLeft) {
			if(mState == ResultsState.HISTORY) {
				loadSession(BenchmarkTestModel.BEST_SESSION_ID);
				AnimatePageChange(mState, ResultsState.BEST);
				
			} else if (mState == ResultsState.RESULT) {
				AnimatePageChange(mState, ResultsState.HISTORY);
				
			} else if (mState == ResultsState.INFO && !Utils.isTablet(getActivity())) {
				loadSession(mSelectedSessionId);
				AnimatePageChange(mState, mInfoFromState);
				
			}
			
		} else if(v.getId() == R.id.headerButtonRight) {
			if (mState == ResultsState.BEST && !Utils.isTablet(getActivity())) {
				AnimatePageChange(mState, ResultsState.HISTORY);
				
			}
			 
		} else if(v.getId() == R.id.results_navbar_extraRightButton) {
			AnimatePageChange(mState, mInfoFromState);
			
		}
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		if(parent.getId() == R.id.results_historyList) {
			if (mState == ResultsState.HISTORY || Utils.isTablet(getActivity())) {
				Session s = (Session) mHistoryList.getItemAtPosition(position);
				if(Utils.isTablet(getActivity())) {
					if(position == 0) {
						loadSession(BenchmarkTestModel.BEST_SESSION_ID);
						BenchmarkApplication.getAppModel().SetFragmentState(this, ResultsState.BEST.name());
						mState = ResultsState.BEST;
					} else {
						loadSession(s.sessionId());
						BenchmarkApplication.getAppModel().SetFragmentState(this, ResultsState.RESULT.name());
						mState = ResultsState.RESULT;
					}
					mSelectedResultTitle = DateFormat.getDateTimeInstance().format(new Date(s.sessionId())).toString();
					mTestList.setSelection(0);
					setupState();
					setupText();
				} else {
					loadSession(s.sessionId());
					mSelectedResultTitle = DateFormat.getDateTimeInstance().format(new Date(s.sessionId())).toString();
					AnimatePageChange(mState, ResultsState.RESULT);
				}
			}
			
		} else if(parent.getId() == R.id.results_testList) {
			ResultItem item = (ResultItem)mTestList.getAdapter().getItem(position);

			if(((BenchmarkApplication)getActivity().getApplication()).isDetailDiagramCompatible()) {
				if(getPageChangeRequestListener() != null && item.status() == Result.Status.OK ) {
					BenchmarkApplication.getAppModel().SetFragmentParams(ResultsFragment.class.toString(), "" + mSelectedSessionId);
					getActivity().startActivity(BatteryActivity.createBatteryIntent(getActivity(), item));
				}
				
			} else {
				if(item.unit().equals("chart") || item.resultId().contains("diagram")) {
					BenchmarkApplication.getAppModel().SetFragmentParams(ResultsFragment.class.toString(), "" + mSelectedSessionId);
					getActivity().startActivity(BatteryActivity.createBatteryIntent(getActivity(), item));
				} else if(getPageChangeRequestListener() != null) {
					BenchmarkApplication.getAppModel().SetFragmentParams(CompareFragment.class.toString(), item.testId() + ", " + item.resultId());
					getPageChangeRequestListener().ChangePage(2);
				}
			}
		}
	}
	


	@Override
	public boolean onItemLongClick(AdapterView<?> parent, View view,
			int position, long id) {
		if(parent.getId() == R.id.results_testList) {
			ResultItem item = (ResultItem)mTestList.getAdapter().getItem(position);
			if(!item.isFirstInGroup()) {

				String imageName = item.testId() + "_full";
				int img_id = getActivity().getResources().getIdentifier(
						imageName, "drawable", getActivity().getApplication().getPackageName());
				if(img_id == 0) img_id = R.drawable.dummy_icon;
				mInfoTitle = Localizator.getString(getActivity(), item.testInfo().testName());
				mInfoText.setText(Localizator.getString(getActivity(), item.description()));
				mInfoImage.setImageResource(img_id);
				
				// We only need the animation if it's really a state change.
				if(mState != ResultsState.INFO) {
					mInfoFromState = mState;
					AnimatePageChange(mState, ResultsState.INFO);
				} else {
					setupState();
					setupText();
				}
				
				return true;
			}
		}
		return false;
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position,
			long id) {
//		if(parent.getId() == R.id.results_testList) {
//			mTestList.refreshVisibleItems();
//		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
//		if(parent.getId() == R.id.results_testList) {
//			mTestList.refreshVisibleItems();
//		}
	}
	
	
	//------------------------------------------------------------------------------------------------------------
	// Animation
	//-----------------------------------------------------------------------------------------------------------
	
	private class AnimationState {
//		public ResultsState fromState;
		public ResultsState toState;
		
		public AnimationState(ResultsState from, ResultsState to) {
//			fromState = from;
			toState = to;
		}
	}
	
	/**
	 * Animation function that animate the state changes to respond the button clicks.
	 * Also changes the current state based on it's params.
	 * @param fromState The page state we were when the animation was requested.
	 * @param toState The page state we want to be in after the animation.
	 */
	private void AnimatePageChange(final ResultsState fromState, final ResultsState toState) {
		if(mAnimState == null) {
			
			if(fromState == ResultsState.BEST && toState == ResultsState.HISTORY && !Utils.isTablet(getActivity())) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.VISIBLE);
				
				mTestList.startAnimation(mAnim_PageLeftOut);
				mHistoryList.startAnimation(mAnim_PageRightIn);
				
			} else if(fromState == ResultsState.HISTORY && toState == ResultsState.BEST && !Utils.isTablet(getActivity())) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.VISIBLE);
				
				mTestList.startAnimation(mAnim_PageLeftIn);
				mHistoryList.startAnimation(mAnim_PageRightOut);
			
			} else if(fromState == ResultsState.RESULT && toState == ResultsState.HISTORY && !Utils.isTablet(getActivity())) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.VISIBLE);
				
				mTestList.startAnimation(mAnim_PageRightOut);
				mHistoryList.startAnimation(mAnim_PageLeftIn);
				
			} else if(fromState == ResultsState.HISTORY && toState == ResultsState.RESULT && !Utils.isTablet(getActivity())) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mHistoryList.setVisibility(View.VISIBLE);
				
				mTestList.startAnimation(mAnim_PageRightIn);
				mHistoryList.startAnimation(mAnim_PageLeftOut);
			
			} else if((fromState == ResultsState.BEST || fromState == ResultsState.RESULT) && toState == ResultsState.INFO) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.VISIBLE);
				
				if(Utils.isTablet(getActivity())) {
					onAnimationEnd(mAnim_PageLeftOut);
				} else {
					mTestList.startAnimation(mAnim_PageLeftOut);
					mInfoBack.startAnimation(mAnim_PageRightIn);
				}
				
			} else if(fromState == ResultsState.INFO && (toState == ResultsState.BEST || toState == ResultsState.RESULT)) {
				mAnimState = new AnimationState(fromState, toState);
				mTestList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.VISIBLE);

				if(Utils.isTablet(getActivity())) {
					onAnimationEnd(mAnim_PageLeftOut);
				} else {
					mTestList.startAnimation(mAnim_PageLeftIn);
					mInfoBack.startAnimation(mAnim_PageRightOut);
				}
			}
		}
	}

	@Override
	public void onAnimationEnd(Animation animation) {
		if((animation == mAnim_PageRightOut || 
				animation == mAnim_PageRightIn ||
				animation == mAnim_PageLeftOut ||
				animation == mAnim_PageLeftIn)
				&& mAnimState != null) {
			BenchmarkApplication.getAppModel().SetFragmentState(this, mAnimState.toState.name());
			mState = mAnimState.toState;
			setupState();
			setupText();
			mAnimState = null;
		}
	}

	@Override
	public void onAnimationRepeat(Animation animation) {
	}

	@Override
	public void onAnimationStart(Animation animation) {
	}
	

	//------------------------------------------------------------------------------------------------------------
	// BaseFragment overrides
	//-----------------------------------------------------------------------------------------------------------

	@Override
	public boolean HandleBackButton() {
		boolean handled = false;
		if(mState == ResultsState.INFO) {
			AnimatePageChange(mState, mInfoFromState);
			handled = true;
		
		} else if(mState == ResultsState.RESULT && !Utils.isTablet(getActivity())) {
			AnimatePageChange(mState, ResultsState.HISTORY);
			handled = true;
		
		} else if(mState == ResultsState.HISTORY) {
			loadSession(BenchmarkTestModel.BEST_SESSION_ID);
			AnimatePageChange(mState, ResultsState.BEST);
			handled = true;
		
		} else if(getPageChangeRequestListener() != null) {
			BenchmarkApplication.getAppModel().SetFragmentState(HomeFragment.class.toString(), HomeState.NORMAL.name());
			getPageChangeRequestListener().ChangePage(0);
			handled = true;
		}
		
		return handled;
	}

	@Override
	public String getDefaultStateString() {
		return ResultsState.BEST.name();
	}

	@Override
	public void pageStateChanged() {
		mState = ResultsState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
		refreshModel();
	}
}
