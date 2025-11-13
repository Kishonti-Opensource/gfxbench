/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Point;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLayoutChangeListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.MainActivity;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.fragments.CompareFragment.CompareState;
import net.kishonti.benchui.lists.testselect.TestListDataProvider;
import net.kishonti.benchui.lists.testselect.TestSelectAdapter;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.benchui.model.ResultsChangedListener;
import net.kishonti.customcomponents.CirclesControl;
import net.kishonti.swig.TestItem;
import net.kishonti.swig.TestItemVector;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListView;

public class HomeFragment extends BaseFragment implements 
	OnClickListener, OnItemClickListener, OnItemLongClickListener,
	AnimationListener, ResultsChangedListener {

	/**
	 * Root views and backgrounds.
	 */
	private View mRootView;
	private View mTestSelectorBack;
	private View mMainBack;
	private View mInfoBack;
	
	/**
	 * Navigation
	 */
	private Button mNavbarLeft;
	private Button mNavbarRight;
	private TextView mNavbarTitle;
	
	private Button mPlusNavbarRight;
	private TextView mPlusNavbarTitle;
	
	/**
	 * Main controls
	 */
//	private ListView mTestSelectorList;
	private TextView mVersionText;
	private CirclesControl mCircleControl;
	private ImageView mInfoImage;
	private TextView mInfoText;
	private Spinner mSpinner;
	
	/**
	 * Button handlers
	 */
	private MainCircleHandler mMainHandler;
	private SideCircleHandler mSideHandler;
	private boolean startLocked = false;
	
	/**
	 * Animation
	 */
	public enum HomeState {
		NORMAL, TEST_SELECT, INFO
	}
	private HomeState mState = HomeState.NORMAL;
	private Animation mAnim_NormalToSelect_NormalIn;
	private Animation mAnim_NormalToSelect_NormalOut;
	private Animation mAnim_NormalToSelect_SelectIn;
	private Animation mAnim_NormalToSelect_SelectOut;
	private Animation mAnim_SelectToInfo_SelectIn;
	private Animation mAnim_SelectToInfo_SelectOut;
	private Animation mAnim_SelectToInfo_InfoIn;
	private Animation mAnim_SelectToInfo_InfoOut;
	private AnimationState mAnimState;
	
	// used for tablet animation
	private int mRootWidth = 0;
	private float mMainOffset = 0;
	private String mInfoTitle = "";
	
	
	private TestSelectAdapter mAdapter;
	private ThemeListView mTestSelectorList;
	
	private static final String RunaloneSelectionTitle = "StabilityTestTitle";
	private static final String RunaloneSelectionBody = "StabilityTestBody";
	private static final String RunaloneSelectionPositive = "Yes";
	private static final String RunaloneSelectionNegative = "No";
	
	/**
	 * Lifetime functions
	 */
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
//		mRootView = inflater.inflate(R.layout.fragment_home, container, false);
		mRootView = inflater.inflate(Utils.getLayoutId(getActivity(), "fragment_home"), container, false);
	    
		BenchmarkApplication.getAppModel().SetFragmentState(this, mState.name());
	    
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
						mRootWidth = size.x > size.y ? size.x : size.y;
						mMainOffset = (int)(mRootWidth * (1.0f/3.0f));
						setupState();
					}
				}
			});
		}
		               
		mTestSelectorBack = (View)mRootView.findViewById(R.id.main_testSelectorBack);
		mMainBack = (View)mRootView.findViewById(R.id.main_homeBack);
		mInfoBack = (View)mRootView.findViewById(R.id.main_infoBack);
        
		mTestSelectorList = (ThemeListView)mRootView.findViewById(R.id.main_testSelectListView);
		mVersionText = (TextView)mRootView.findViewById(R.id.main_versionText);
		mCircleControl = (CirclesControl)mRootView.findViewById(R.id.main_circleControl);
		mInfoImage = (ImageView)mRootView.findViewById(R.id.main_infoImage);
		mInfoText = (TextView)mRootView.findViewById(R.id.main_infoText);
		
		mSpinner = (Spinner) mRootView.findViewById(R.id.main_clDeviceSelector);
		mNavbarLeft = (Button)mRootView.findViewById(R.id.headerButtonLeft);
		mNavbarRight = (Button)mRootView.findViewById(R.id.headerButtonRight);
		mNavbarTitle = (TextView)mRootView.findViewById(R.id.headerTitle);
		
		if(Utils.isTablet(getActivity())) {
			mPlusNavbarRight = (Button)mRootView.findViewById(R.id.main_secondary_navbar_right);
			mPlusNavbarTitle = (TextView)mRootView.findViewById(R.id.main_secondary_navbar_title);
		}
		
		// Setup the button handlers
		mNavbarLeft.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		if(Utils.isTablet(getActivity())) {
			mPlusNavbarRight.setOnClickListener(this);
		}
		mMainHandler = new MainCircleHandler();
		mSideHandler = new SideCircleHandler();
		mCircleControl.setMainOnTouchHandler(mMainHandler);
		mCircleControl.setSideOnTouchHandler0(mSideHandler);
	    
	    mAnim_NormalToSelect_NormalIn = AnimationUtils.loadAnimation(getActivity(), R.anim.home_main_from_select);
	    mAnim_NormalToSelect_NormalOut = AnimationUtils.loadAnimation(getActivity(), R.anim.home_main_to_select);
	    mAnim_NormalToSelect_SelectIn = AnimationUtils.loadAnimation(getActivity(), R.anim.home_select_from_main);
	    mAnim_NormalToSelect_SelectOut = AnimationUtils.loadAnimation(getActivity(), R.anim.home_select_to_main);
	    mAnim_SelectToInfo_InfoIn = AnimationUtils.loadAnimation(getActivity(), R.anim.home_info_from_select);
	    mAnim_SelectToInfo_InfoOut = AnimationUtils.loadAnimation(getActivity(), R.anim.home_info_to_select);
	    mAnim_SelectToInfo_SelectIn = AnimationUtils.loadAnimation(getActivity(), R.anim.home_select_from_info);
	    mAnim_SelectToInfo_SelectOut = AnimationUtils.loadAnimation(getActivity(), R.anim.home_select_to_info);
	    
	    mAnim_NormalToSelect_NormalIn.setAnimationListener(this);
	    mAnim_NormalToSelect_NormalOut.setAnimationListener(this);
	    mAnim_NormalToSelect_SelectIn.setAnimationListener(this);
	    mAnim_NormalToSelect_SelectOut.setAnimationListener(this);
	    mAnim_SelectToInfo_InfoIn.setAnimationListener(this);
	    mAnim_SelectToInfo_InfoOut.setAnimationListener(this);
	    mAnim_SelectToInfo_SelectIn.setAnimationListener(this);
	    mAnim_SelectToInfo_SelectOut.setAnimationListener(this);
	    
	    mTestSelectorList.setDescendantFocusability(ViewGroup.FOCUS_AFTER_DESCENDANTS);
	    mTestSelectorList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
	    mTestSelectorList.setOnItemClickListener(this);
	    mTestSelectorList.setOnItemLongClickListener(this);

	    mCircleControl.setNextFocusRightId(R.id.main_testSelectListView);
	    mCircleControl.setNextFocusForwardId(R.id.main_testSelectListView);
	    mCircleControl.setNextFocusUpId(R.id.main_testSelectListView);
	    mCircleControl.setNextFocusDownId(R.id.main_testSelectListView);
	    
	    mNavbarLeft.setNextFocusLeftId(R.id.main_circleControl);
	    mTestSelectorList.setNextFocusLeftId(R.id.main_circleControl);
	    
	    
		return mRootView;
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		BenchmarkTestModel model = BenchmarkApplication.getModel(getActivity());
		model.loadTestSelection("prefs");
		
		mState = HomeState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
		
		startLocked = false;
		
		setupState();
		setupText();
		
		Context context = this.getActivity();
		
		String version = ((BenchmarkApplication)BenchmarkApplication.instance).getVersionString();
		mVersionText.setText(version);
		if(mState==HomeState.NORMAL || !Utils.isTablet(getActivity())) {
			mCircleControl.setMainText(Localizator.getString(context,  "StartAll"));
			mCircleControl.setSideText(Localizator.getString(context, "TestSelection"));
		}
		if(mState==HomeState.TEST_SELECT && Utils.isTablet(getActivity())) {
			mCircleControl.setMainText(Localizator.getString(context, "Start"));
			mCircleControl.setSideText(Localizator.getString(context, "Back"));
		}
		
		// Setup adapters based on model if application is initialized
		if(((BenchmarkApplication)getActivity().getApplication()).mIsInitialized) {
	    	model.loadTestSelection("prefs");
	    	model.registerResultChangedListener(this);
		    
		    mAdapter = new TestSelectAdapter(getActivity(), model);
			mTestSelectorList.setAdapter(mAdapter);
			mAdapter.notifyDataSetChanged();
			
			List<String> openclConfigs = net.kishonti.systeminfo.DataCollector.getCBJNIOpenCLDeviceList();
			if (openclConfigs.size() == 0) {
				mSpinner.setVisibility(View.GONE);
			} else {
				ArrayAdapter<String> deviceAdapter = new ArrayAdapter<String>(getActivity(), R.layout.layout_spinner_clconfig_select, openclConfigs);
				mSpinner.setAdapter(deviceAdapter);
		        SharedPreferences pref = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE);
		        mSpinner.setSelection(pref.getInt("selectedOpenclConfigIndex", 0), false);
		        mSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
		
					@Override
					public void onItemSelected(AdapterView<?> arg0, View arg1, int position, long id) {
		                SharedPreferences pref = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE);
		                Editor e = pref.edit();
		                e.putInt("selectedOpenclConfigIndex", position);
		                e.commit();
					}
		
					@Override
					public void onNothingSelected(AdapterView<?> arg0) {
		                SharedPreferences pref = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE);
		                Editor e = pref.edit();
		                e.putInt("selectedOpenclConfigIndex", 0);
		                e.commit();				
					}
				});
			}
		}
		
		
	}
	
	@Override
	public void onPause() {
		// Save the selected tests
		BenchmarkApplication.getModel(getActivity()).saveTestSelection("prefs");
		
		// Erase the adapters, model specific things
		mAdapter = null;
		
		if(mTestSelectorList != null)
			mTestSelectorList.setAdapter(null);
		
		if(mSpinner != null) {
			mSpinner.setAdapter(null);
			mSpinner.setOnItemSelectedListener(null);
		}
		
		super.onPause();
	}
	
	/**
	 * Manages the state change visibility.
	 */
	private void setupState() {
		if(mState == HomeState.NORMAL) {
			if(Utils.isTablet(getActivity())) {
		    	RelativeLayout.LayoutParams mainBackParams = (RelativeLayout.LayoutParams) mMainBack.getLayoutParams();
		    	mainBackParams.width = (int)mMainOffset;
		    	mMainBack.setLayoutParams(mainBackParams);
		    	mMainBack.setX(mMainOffset);
		    	mMainBack.invalidate();
		    	mTestSelectorBack.setX(mRootWidth);
		    	mTestSelectorBack.invalidate();
		    	mCircleControl.setMainText(Localizator.getString( this.getActivity() , "StartAll"));
				mCircleControl.setSideText(Localizator.getString( this.getActivity(), "TestSelection"));
				
				mNavbarLeft.setVisibility(View.VISIBLE);
			}
			
			mMainBack.setVisibility(View.VISIBLE);
			mTestSelectorBack.setVisibility(View.GONE);
			mTestSelectorList.setVisibility(View.GONE);
			mInfoBack.setVisibility(View.GONE);
			
		} else if(mState == HomeState.TEST_SELECT) {
			if(Utils.isTablet(getActivity())) {
		    	mMainBack.setX(0);
		    	mMainBack.invalidate();
		    	mTestSelectorBack.setX(mMainOffset);
		    	mTestSelectorBack.invalidate();
		    	mCircleControl.setMainText(Localizator.getString( this.getActivity() , "Start"));
				mCircleControl.setSideText(Localizator.getString( this.getActivity(), "Back"));

				mNavbarLeft.setVisibility(View.VISIBLE);
			}
			
			mMainBack.setVisibility(Utils.isTablet(getActivity()) ? View.VISIBLE : View.GONE);
			mTestSelectorBack.setVisibility(View.VISIBLE);
			mTestSelectorList.setVisibility(View.VISIBLE);
			mInfoBack.setVisibility(Utils.isTablet(getActivity()) ? View.INVISIBLE : View.GONE);
			
		} else if(mState == HomeState.INFO) {
			if(Utils.isTablet(getActivity())) {
				mMainBack.setX(-mMainOffset);
				mMainBack.invalidate();
		    	mTestSelectorBack.setX(0);
		    	mTestSelectorBack.invalidate();

				mNavbarLeft.setVisibility(View.GONE);
			}
			
			mMainBack.setVisibility(View.GONE);
			mTestSelectorBack.setVisibility(View.VISIBLE);
			mTestSelectorList.setVisibility(Utils.isTablet(getActivity()) ? View.VISIBLE : View.GONE);
			mInfoBack.setVisibility(View.VISIBLE);
		}
	}
	
	/**
	 * Setup state dependent texts
	 */
	private void setupText() {
		if(!Utils.isTablet(getActivity())) {
			if(mState == HomeState.INFO) {
				mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Back"));
				mNavbarTitle.setText(Localizator.getString(this.getActivity(), mInfoTitle));
				mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
				
			} else {
				mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Back"));
				mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TestSelection"));
				mNavbarRight.setText(Localizator.getString(this.getActivity(), "Start"));
				
			}
			
		} else {
			if(mState == HomeState.INFO) {
				mNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TestSelection"));
				mNavbarRight.setText(Localizator.getString(this.getActivity(), "Start"));

				mPlusNavbarTitle.setText(Localizator.getString(this.getActivity(), mInfoTitle));
				mPlusNavbarRight.setText(Localizator.getString(this.getActivity(), "Close"));
				
			} else {
				mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Close"));
				mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TestSelection"));
				mNavbarRight.setText(Localizator.getString(this.getActivity(), "Start"));

				mPlusNavbarTitle.setText(Localizator.getString(this.getActivity(), ""));
				mPlusNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
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
			if(mState == HomeState.TEST_SELECT) {
				AnimatePageChange(mState, HomeState.NORMAL);
				
			} else if (mState == HomeState.INFO && !Utils.isTablet(getActivity())) {
				AnimatePageChange(mState, HomeState.TEST_SELECT);
			}
			
		} else if(v.getId() == R.id.headerButtonRight) {
			if (mState != HomeState.NORMAL) {
				startButton();
			}
		} else if(v.getId() == R.id.main_secondary_navbar_right) {
			if (mState == HomeState.INFO) {
				AnimatePageChange(mState, HomeState.TEST_SELECT);
			}
		}
	}
	
	private void startButton() {
		TestItemVector tests = BenchmarkApplication.getModel(getActivity()).getSelectedTests(false);
		List<String> selected = new ArrayList<String>();
		for (int i = 0; i < tests.size(); ++i) {
			selected.add(tests.get(i).testId());
		}
		 
		if (selected.size() == 0) {
			Toast.makeText(getActivity().getApplicationContext(), 
					"Select some tests first!", Toast.LENGTH_LONG).show();
			
		} else {
			if(!startLocked) {
				startLocked = true;
				startSelected(selected);
			}
		}
	}
	
	private void startSelected(List<String> selectedtests) {
		try {
			((MainActivity) getActivity()).startTestSession(selectedtests);
		} catch (Exception e) {
			startLocked = false;
			Toast.makeText(getActivity().getApplicationContext(), 
					"Test start failed!", Toast.LENGTH_LONG).show();
			e.printStackTrace();
		}
	}
	
	/**
	 * The CircleControl's main button handler
	 * @author kishonti
	 */
	public class MainCircleHandler implements Runnable {
		@Override
		public void run() {
			if(mState == HomeState.NORMAL) {
				TestItemVector tests = BenchmarkApplication.getModel(getActivity()).getSelectedTests(true);
				List<String> selected = new ArrayList<String>();
				for (int i = 0; i < tests.size(); ++i) {
					selected.add(tests.get(i).testId());
				}
				startSelected(selected);
			}
			if(mState == HomeState.TEST_SELECT)
				startButton();
		}
	}	
	
	/**
	 * The CircleControl's side button handler
	 * @author kishonti
	 */
	public class SideCircleHandler implements Runnable {
		@Override
		public void run() {
			if(mState == HomeState.NORMAL) {
				AnimatePageChange(mState, HomeState.TEST_SELECT);
			} else {
				AnimatePageChange(mState, HomeState.NORMAL);
			}
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------------------
	// Animation
	//-----------------------------------------------------------------------------------------------------------
	
	/**
	 * Animation function that animate the state changes to respond the button clicks.
	 * Also changes the current state based on it's params.
	 * @param fromState The page state we were when the animation was requested.
	 * @param toState The page state we want to be in after the animation.
	 */
	private void AnimatePageChange(final HomeState fromState, final HomeState toState) {
		if(mAnimState == null) {
			
			if(fromState == HomeState.NORMAL && toState == HomeState.TEST_SELECT) {
				mAnimState = new AnimationState(fromState, toState);
				mMainBack.setVisibility(View.VISIBLE);
				mTestSelectorBack.setVisibility(View.VISIBLE);
				mTestSelectorList.setVisibility(View.VISIBLE);
				
				if(Utils.isTablet(getActivity())) {
			    	mMainBack.setX(0);
			    	mMainBack.invalidate();
			    	mTestSelectorBack.setX(mMainOffset);
			    	mTestSelectorBack.invalidate();
				}
				
				mMainBack.startAnimation(mAnim_NormalToSelect_NormalOut);
				mTestSelectorBack.startAnimation(mAnim_NormalToSelect_SelectIn);
				if(!Utils.isTablet(getActivity())) mTestSelectorList.startAnimation(mAnim_NormalToSelect_SelectIn);
				
			} else if(fromState == HomeState.TEST_SELECT && toState == HomeState.NORMAL) {
				mAnimState = new AnimationState(fromState, toState);
				mMainBack.setVisibility(View.VISIBLE);
				mTestSelectorBack.setVisibility(View.VISIBLE);
				mTestSelectorList.setVisibility(View.VISIBLE);
				
				if(Utils.isTablet(getActivity())) {
			    	mMainBack.setX(mMainOffset);
			    	mMainBack.invalidate();
			    	mTestSelectorBack.setX(mRootWidth);
			    	mTestSelectorBack.invalidate();
				}

				mMainBack.startAnimation(mAnim_NormalToSelect_NormalIn);
				mTestSelectorBack.startAnimation(mAnim_NormalToSelect_SelectOut);
				if(!Utils.isTablet(getActivity())) mTestSelectorList.startAnimation(mAnim_NormalToSelect_SelectOut);
			
			} else if(fromState == HomeState.INFO && toState == HomeState.TEST_SELECT) {
				mAnimState = new AnimationState(fromState, toState);
				mTestSelectorList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.VISIBLE);
				
				if(Utils.isTablet(getActivity())) {
					mMainBack.setX(0);
					mMainBack.invalidate();
			    	mTestSelectorBack.setX(mMainOffset);
			    	mTestSelectorBack.invalidate();
			    	
					mMainBack.setVisibility(View.VISIBLE);
					mMainBack.startAnimation(mAnim_SelectToInfo_SelectIn);
					mTestSelectorBack.startAnimation(mAnim_SelectToInfo_SelectIn);
				} else {
					mInfoBack.startAnimation(mAnim_SelectToInfo_InfoOut);
					mTestSelectorList.startAnimation(mAnim_SelectToInfo_SelectIn);
				}
				
			} else if(fromState == HomeState.TEST_SELECT && toState == HomeState.INFO) {
				mAnimState = new AnimationState(fromState, toState);
				mTestSelectorList.setVisibility(View.VISIBLE);
				mInfoBack.setVisibility(View.VISIBLE);

				
				if(Utils.isTablet(getActivity())) {
					mMainBack.setVisibility(View.VISIBLE);
					mMainBack.setX(-mMainOffset);
					mMainBack.invalidate();
			    	mTestSelectorBack.setX(0);
			    	mTestSelectorBack.invalidate();
			    	
					mMainBack.startAnimation(mAnim_SelectToInfo_SelectOut);
					mTestSelectorBack.startAnimation(mAnim_SelectToInfo_SelectOut);
				} else {
					mInfoBack.startAnimation(mAnim_SelectToInfo_InfoIn);
					mTestSelectorList.startAnimation(mAnim_SelectToInfo_SelectOut);
				}
			}
		}
	}

	@Override
	public void onAnimationEnd(Animation animation) {
		if((animation == mAnim_NormalToSelect_NormalOut || 
				animation == mAnim_NormalToSelect_SelectOut || 
				animation == mAnim_SelectToInfo_InfoOut ||
				animation == mAnim_SelectToInfo_SelectOut ||
				animation == mAnim_SelectToInfo_SelectIn )
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
	
	private class AnimationState {
//		public HomeState fromState;
		public HomeState toState;
		
		public AnimationState(HomeState from, HomeState to) {
//			fromState = from;
			toState = to;
		}
	}


	//------------------------------------------------------------------------------------------------------------
	// BaseFragment overrides
	//-----------------------------------------------------------------------------------------------------------

	@Override
	public boolean HandleBackButton() {
		if (mState == HomeState.TEST_SELECT) {
			AnimatePageChange(mState, HomeState.NORMAL);
			return true; 
			
		} else if (mState == HomeState.INFO) {
			AnimatePageChange(mState, HomeState.TEST_SELECT);
			return true; 
		}
		
		return false;
	}
	
	@Override
	public void pageStateChanged() {
		mState = HomeState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));

		setupState();
		setupText();
	}

	@Override
	public String getDefaultStateString() {
		return HomeState.NORMAL.name();
	}

	@Override
	public boolean onItemLongClick(AdapterView<?> parent, View view,
			int position, long id) {
		if(parent.getId() == R.id.main_testSelectListView) {
			TestItem item = ((TestListDataProvider)BenchmarkApplication.getModel(getActivity())).getTestForPosition(position);
			if(!item.isFirstInGroup()) {

				String imageName = item.testId() + "_full";
				int img_id = getActivity().getResources().getIdentifier(
						imageName, "drawable", getActivity().getApplication().getPackageName());
				if(img_id == 0) img_id = R.drawable.dummy_icon;
				mInfoTitle = Localizator.getString(getActivity(), item.testInfo().testName());
				mInfoText.setText(Localizator.getString(getActivity(), item.description()));
				mInfoImage.setImageResource(img_id);
				
				// We only need the animation if it's really a state change.
				if(mState == HomeState.TEST_SELECT) {
					AnimatePageChange(mState, HomeState.INFO);
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
	public void onItemClick(AdapterView<?> parent, View view, int position,
			long id) {
		if(parent.getId() == R.id.main_testSelectListView) {
			final TestItem item = ((TestListDataProvider)BenchmarkApplication.getModel(getActivity())).getTestForPosition(position);
			
			if(item.requires("runalone") && !item.isSelected()) {
				
				if(item.isFirstInGroup() && !item.isGroupSelectionEnabled()) {
					return;
				}
				
				AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
				builder.setTitle(Localizator.getString(getActivity(), HomeFragment.RunaloneSelectionTitle));
				builder.setMessage(Localizator.getString(getActivity(), HomeFragment.RunaloneSelectionBody));
				builder.setCancelable(false);
				builder.setPositiveButton(HomeFragment.RunaloneSelectionPositive,
		                new DialogInterface.OnClickListener() {
			                public void onClick(DialogInterface dialog, int id) {
			                	if(item.isFirstInGroup()) {
			                		BenchmarkApplication.getModel(getActivity()).setSelectionForGroup(item.groupId(), !item.isGroupSelected());
			        			} else {
			        				BenchmarkApplication.getModel(getActivity()).setSelectionForTest(item.testId(), !item.isSelected());
			        			}
			                    dialog.dismiss();
			                    BenchmarkApplication.getModel(getActivity()).getTestList();
			    				mTestSelectorList.refreshVisibleItems();
			                }
			            });
	            builder.setNegativeButton(HomeFragment.RunaloneSelectionNegative,
		            new DialogInterface.OnClickListener() {
		                public void onClick(DialogInterface dialog, int id) {
		                    dialog.cancel();
		    				mTestSelectorList.refreshVisibleItems();
		                }
		            });

	            AlertDialog alert = builder.create();
	            alert.show();
	            
			} else {
				
				if(item.isFirstInGroup()) {
					BenchmarkApplication.getModel(getActivity()).setSelectionForGroup(item.groupId(), !item.isGroupSelected());
				} else {
					BenchmarkApplication.getModel(getActivity()).setSelectionForTest(item.testId(), !item.isSelected());
				}
				BenchmarkApplication.getModel(getActivity()).getTestList();
				mTestSelectorList.refreshVisibleItems();
			}
			
		}
	}

	@Override
	public void HandleResultsChanged() {
		if(mAdapter != null)
			mAdapter.notifyDataSetChanged();
	}
	
}
