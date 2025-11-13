/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Animation.AnimationListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.db.CompareSQLiteHelper.FormFactorFilter;
import net.kishonti.benchui.fragments.HomeFragment.HomeState;
import net.kishonti.benchui.fragments.interfaces.HideKeyboardListener;
import net.kishonti.benchui.lists.resultlist.CompareListAdapter;
import net.kishonti.benchui.lists.resultlist.DuelAdapter;
import net.kishonti.benchui.lists.resultlist.ResultFormatter;
import net.kishonti.benchui.lists.resultlist.ResultSelectionListAdapter;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.benchui.model.ResultsChangedListener;
import net.kishonti.benchui.resultsdb.ResultsDAODatabase;
import net.kishonti.swig.CompareResult;
import net.kishonti.swig.CompareResultVector;
import net.kishonti.swig.ResultItem;
import net.kishonti.swig.ResultItemVector;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListView;
import gfxc.dao.Result;

import java.util.HashMap;
import java.util.List;
import de.greenrobot.dao.query.LazyList;

public class CompareFragment extends BaseFragment implements
	OnClickListener, OnItemClickListener,
	HideKeyboardListener, AnimationListener, ResultsChangedListener {

	String active_filter = "";

	/**
	 * Root views and backgrounds.
	 */
	private View mRootView;
	private View mDeviceListBack;
	private View mDuelListBack;

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
	private ImageView mTestSelectorLeft;
	private ImageView mTestSelectorImage;
	private TextView mTestSelectorName;
	private TextView mTestSelectorDesc;
	private ImageView mTestSelectorRight;

	public ProgressBar mProgressBar;
	public ImageView mDeviceImage;
	public ImageView mArrowimage;
	public TextView mDeviceName;
	public TextView mDeviceDesc;
	public TextView mScore;
	public TextView mUnit;

	private EditText mSearchbar;
	private ThemeListView mTestSelectorList;
	private ThemeListView mDeviceList;
	private ListView mDuelList;
	private TextView mDuelOwnLabel;
	private TextView mDuelOtherLabel;

	private CompareListAdapter mCompareAdapter;
	private DuelAdapter mDuelAdapter;
	private LazyList<Result> mCompareList;
	private ResultSelectionListAdapter mTestSelectAdapter;

	private ResultItem mCurrentTest;

	/**
	 * Animation
	 */
	public enum CompareState {
		COMPARE, DUEL
	}
	private CompareState mState = CompareState.COMPARE;
	private Animation mAnim_PageLeftIn;
	private Animation mAnim_PageLeftOut;
	private Animation mAnim_PageRightIn;
	private Animation mAnim_PageRightOut;
	private AnimationState mAnimState;

	private void refreshModel() {
		boolean isTablet = Utils.isTablet(getActivity());
		BenchmarkApplication.getModel(getActivity()).registerResultChangedListener(this);

		mTestSelectAdapter = new ResultSelectionListAdapter(getActivity(), BenchmarkApplication.getModel(getActivity()), isTablet);
		if(isTablet) {
			mTestSelectorList.setAdapter(mTestSelectAdapter);
			mTestSelectAdapter.notifyDataSetChanged();
		}

		mCurrentTest = (ResultItem)mTestSelectAdapter.getFirstSelectableItem();

		refreshTestSelector();
		refreshCompare();
	}

	private void refreshCompare() {

		boolean showDesktop = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getBoolean("displayDesktop", false);

		if(mCurrentTest != null) {
			mCompareAdapter = new CompareListAdapter(
					getActivity(),
					BenchmarkApplication.getModel(getActivity()),
					mCurrentTest.resultId(),
					showDesktop ? FormFactorFilter.ALL : FormFactorFilter.MOBILE,
					mSearchbar != null ? mSearchbar.getText().toString() : "");
			mDeviceList.setAdapter(mCompareAdapter);
			mCompareAdapter.notifyDataSetChanged();

			refreshTestSelector();
		}
	}

	@Override
	public void HandleResultsChanged() {
		if(getActivity() == null) return;

		BenchmarkApplication.getAppModel().SetFragmentState(this, CompareState.COMPARE.name());
		mState = CompareState.COMPARE;
		setupState();
		setupText();
		refreshModel();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
//		mRootView = inflater.inflate(R.layout.fragment_compare, container, false);
		mRootView = inflater.inflate(Utils.getLayoutId(getActivity(), "fragment_compare"), container, false);

		BenchmarkApplication.getAppModel().SetFragmentState(this, mState.name());

		mDeviceListBack = (View)mRootView.findViewById(R.id.compare_deviceListBack);
		mDuelListBack = (View)mRootView.findViewById(R.id.compare_duelListBack);

		mNavbarLeft = (Button)mRootView.findViewById(R.id.headerButtonLeft);
		mNavbarRight = (Button)mRootView.findViewById(R.id.headerButtonRight);
		mNavbarTitle = (TextView)mRootView.findViewById(R.id.headerTitle);

		mSearchbar = (EditText)mRootView.findViewById(R.id.compare_searchbar);
		mDuelOwnLabel = (TextView)mRootView.findViewById(R.id.duel_ownLabel);
		mDuelOtherLabel = (TextView)mRootView.findViewById(R.id.duel_otherLabel);

		mDeviceList = (ThemeListView)mRootView.findViewById(R.id.compare_deviceList);
		mDuelList = (ListView)mRootView.findViewById(R.id.compare_duelList);

		if(Utils.isTablet(getActivity())) {
			mTestSelectorList = (ThemeListView)mRootView.findViewById(R.id.compare_testSelectorList);
			mTestSelectorList.setOnItemClickListener(this);

			mExtraNavbarBack = mRootView.findViewById(R.id.compare_navbar_extraBack);
			mExtraNavbarLeft = (Button)mRootView.findViewById(R.id.compare_navbar_extraLeftButton);
			mExtraNavbarRight = (Button)mRootView.findViewById(R.id.compare_navbar_extraRightButton);
			mExtraNavbarTitle = (TextView)mRootView.findViewById(R.id.compare_navbar_extraTitle);

			mExtraNavbarLeft.setOnClickListener(this);
			mExtraNavbarRight.setOnClickListener(this);

		} else {
			// test navigation bar
			mTestSelectorImage = (ImageView)mRootView.findViewById(R.id.testIcon);
			mTestSelectorName = (TextView)mRootView.findViewById(R.id.testName);
			mTestSelectorDesc = (TextView)mRootView.findViewById(R.id.testDescription);
			mTestSelectorLeft = (ImageView)mRootView.findViewById(R.id.compare_testSelectorLeft);
			mTestSelectorRight = (ImageView)mRootView.findViewById(R.id.compare_testSelectorRight);
		}

		//own result
		mProgressBar = (ProgressBar) mRootView.findViewById(R.id.cell_compare_progressBar);
		mDeviceImage = (ImageView) mRootView.findViewById(R.id.cell_compare_deviceImage);
		mArrowimage = (ImageView) mRootView.findViewById(R.id.cell_compare_arrowImage);
		mDeviceName = (TextView) mRootView.findViewById(R.id.cell_compare_deviceName);
		mDeviceDesc = (TextView) mRootView.findViewById(R.id.cell_compare_deviceDesc);
		mScore = (TextView) mRootView.findViewById(R.id.cell_compare_resultScore);
		mUnit = (TextView) mRootView.findViewById(R.id.cell_compare_resultUnit);

		mArrowimage.setVisibility(View.INVISIBLE);
		mDeviceName.setText(Localizator.getString(getActivity(), "YourBestScore"));
		mDuelOwnLabel.setText(Localizator.getString(getActivity(), "YourBestScore"));
		mDeviceDesc.setText("");

		// Setup the action handlers
		mNavbarLeft.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		if(mTestSelectorLeft != null) mTestSelectorLeft.setOnClickListener(this);
		if(mTestSelectorRight != null) mTestSelectorRight.setOnClickListener(this);
		mDeviceList.setOnItemClickListener(this);
		mDuelList.setOnItemClickListener(this);
		mSearchbar.addTextChangedListener(new SearchBarTextWatcher());

	    mAnim_PageRightIn = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_right_in);
	    mAnim_PageRightOut = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_right_out);
	    mAnim_PageLeftIn = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_left_in);
	    mAnim_PageLeftOut = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_left_out);

	    mAnim_PageRightIn.setAnimationListener(this);
	    mAnim_PageRightOut.setAnimationListener(this);
	    mAnim_PageLeftIn.setAnimationListener(this);
	    mAnim_PageLeftOut.setAnimationListener(this);

		return mRootView;
	}

	@Override
	public void onResume() {
		super.onResume();

		mState = CompareState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));

		setupState();
		setupText();

		if(((BenchmarkApplication)getActivity().getApplication()).mIsInitialized) {
			refreshModel();
		}
	}

	@Override
	public void onPause() {
		hideKeyboard();

		BenchmarkApplication.getModel(getActivity()).unRegisterResultChangedListener(this);
		if(mTestSelectorList != null)
			mTestSelectorList.setAdapter(null);
		if(mDeviceList != null)
			mDeviceList.setAdapter(null);

		mCurrentTest = null;
		mTestSelectAdapter = null;

		mCompareList = null;
		mCompareAdapter = null;

		super.onPause();
	}

	@Override
	public void onDestroy() {
		if(mCompareList != null) {
			mCompareList.close();
		}
		super.onDestroy();
	}

	/**
	 * Manages the state change visibility.
	 */
	private void setupState() {
		if(Utils.isTablet(getActivity())) {
			if(mState == CompareState.COMPARE) {
				mDeviceListBack.setVisibility(View.VISIBLE);
				mDuelListBack.setVisibility(View.GONE);
				mTestSelectorList.setVisibility(View.VISIBLE);
				mExtraNavbarBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.GONE);
				mNavbarRight.setVisibility(View.INVISIBLE);

			} else if(mState == CompareState.DUEL) {
				mDeviceListBack.setVisibility(View.VISIBLE);
				mDuelListBack.setVisibility(View.VISIBLE);
				mTestSelectorList.setVisibility(View.GONE);
				mExtraNavbarBack.setVisibility(View.VISIBLE);
				mNavbarLeft.setVisibility(View.GONE);
				mNavbarRight.setVisibility(View.INVISIBLE);

			}

		} else {
			if(mState == CompareState.COMPARE) {
				mDeviceListBack.setVisibility(View.VISIBLE);
				mDuelListBack.setVisibility(View.GONE);
				mNavbarLeft.setVisibility(View.GONE);

			} else if(mState == CompareState.DUEL) {
				mDeviceListBack.setVisibility(View.GONE);
				mDuelListBack.setVisibility(View.VISIBLE);
				mNavbarLeft.setVisibility(View.VISIBLE);

			}
		}
	}

	/**
	 * Setup state dependent texts
	 */
	private void setupText() {
		boolean isTablet = Utils.isTablet(getActivity());
		if(mState == CompareState.COMPARE) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TabCompare"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			if(isTablet) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			}

		} else if(mState == CompareState.DUEL) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), isTablet ? "" : "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TabCompare"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			if(isTablet) {
				mExtraNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
				mExtraNavbarTitle.setText(Localizator.getString(this.getActivity(), "TabDuel"));
				mExtraNavbarRight.setText(Localizator.getString(this.getActivity(), "Close"));
			}

		}
	}



	/**
	 * --------------------------------------------------------------------------------------------------------------
	 * Test selection
	 * --------------------------------------------------------------------------------------------------------------
	 */

	/**
	 * Reloads the picture, name and desc of the test selector and refreshes your own result.
	 */
	public void refreshTestSelector() {
		if(mCurrentTest == null) return;
		if(getActivity() == null) return;
		if(mCompareAdapter == null) return;

		if(!Utils.isTablet(getActivity())) {
			int img_id = getResources().getIdentifier(mCurrentTest.resultId(), "drawable", getActivity().getApplication().getPackageName());

			String text = Localizator.getString(getActivity(), mCurrentTest.description());

			if(mTestSelectorName != null) {
				String title = mCurrentTest.resultId();
				mTestSelectorName.setText(Localizator.getString(getActivity(), title));
				mTestSelectorDesc.setText(text);
				mTestSelectorImage.setImageResource(img_id);
			}
		}

		if(mCurrentTest.score() <= 0 || mCurrentTest.score() > 1000000) {
			mProgressBar.setProgress(0);
			mScore.setText("");
			mScore.setVisibility(View.GONE);
			mUnit.setText(Localizator.getString(getActivity(), "Results_NA"));
		} else {
			String itemUnit = mCurrentTest.unit();
			ResultFormatter.FormattedResult formattedResult =
					new ResultFormatter().getFormattedResult(mCurrentTest.score(), itemUnit);

			int max = 1000;
			int score = (int)(mCurrentTest.score() / mCompareAdapter.getMaxScore() * max);
			mProgressBar.setMax(max);
			mProgressBar.setProgress(score);

			mUnit.setText(itemUnit);
			mScore.setVisibility(View.VISIBLE);
			mScore.setText(formattedResult.getScore());
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
			if(mState == CompareState.DUEL && !Utils.isTablet(getActivity())) {
				AnimatePageChange(mState, CompareState.COMPARE);
			}

		} else if(v.getId() == R.id.compare_testSelectorLeft) {
			int selectedItemIndex = BenchmarkApplication.getModel(getActivity()).getResultSelectorItemPosition(mCurrentTest.resultId());
			mCurrentTest = (ResultItem)mTestSelectAdapter.getItem(mod(selectedItemIndex - 1, mTestSelectAdapter.getCount()));
			refreshTestSelector();
			refreshCompare();

		}  else if(v.getId() == R.id.compare_testSelectorRight) {
			int selectedItemIndex = BenchmarkApplication.getModel(getActivity()).getResultSelectorItemPosition(mCurrentTest.resultId());
			mCurrentTest = (ResultItem)mTestSelectAdapter.getItem(mod(selectedItemIndex + 1, mTestSelectAdapter.getCount()));
			refreshTestSelector();
			refreshCompare();

		} else if(v.getId() == R.id.compare_navbar_extraRightButton) {
			if(mState == CompareState.DUEL) {
				AnimatePageChange(mState, CompareState.COMPARE);
			}

		}
	}

	//Correct modulus calculation.
	private int mod(int x,int d) {
		int m = x % d;
		if (m < 0) m += d;
		return m;
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		if (parent.getId() == R.id.compare_testSelectorList) {
			mCurrentTest = (ResultItem)mTestSelectorList.getItemAtPosition(position);
			refreshCompare();
			mDeviceList.requestFocus();

		} else

		// Selected a device to duel with
		if (parent.getId() == R.id.compare_deviceList) {
			if (mState == CompareState.COMPARE || Utils.isTablet(getActivity())) {
				hideKeyboard();

				CompareResult item = (CompareResult)mDeviceList.getItemAtPosition(position);
				BenchmarkApplication.getModel(getActivity()).getDeviceResults(item.deviceName());
				BenchmarkApplication.getModel(getActivity()).getDuelList();

				mDuelAdapter = new DuelAdapter(getActivity(), BenchmarkApplication.getModel(getActivity()));
				mDuelList.setAdapter(mDuelAdapter);
				mDuelAdapter.notifyDataSetChanged();
				mDuelOtherLabel.setText(item.deviceName());

				AnimatePageChange(CompareState.COMPARE, CompareState.DUEL);
			}

		// selected test on duel screen
		} else

		if(parent.getId() == R.id.compare_duelList) {
			if (mState == CompareState.DUEL) {
				mCurrentTest = BenchmarkApplication.getModel(getActivity()).getDuelList().get(position);
				refreshCompare();
				refreshTestSelector();
				AnimatePageChange(mState, CompareState.COMPARE);
			}

		}
	}

	/**
	 * Search string changed handler
	 */
	private class SearchBarTextWatcher implements TextWatcher {

		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count) {
			if(mCurrentTest != null) {
//				SetCompareTest(s.toString());
				active_filter = s.toString();
				refreshCompare();
			}
		}

		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after) {
		}

		@Override
		public void afterTextChanged(Editable s) {
		}
	}

	@Override
	public void hideKeyboard() {
		try {
			if(getActivity() != null) {
				View focused = getActivity().getCurrentFocus();
				if(focused != null) {
				    InputMethodManager inputMethodManager = (InputMethodManager)  getActivity().getSystemService(Activity.INPUT_METHOD_SERVICE);
				    inputMethodManager.hideSoftInputFromWindow(focused.getWindowToken(), 0);
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			Log.e("Keyboarderror", "the keyboard cannot be hidden se error above", e);
		}
	}



	//------------------------------------------------------------------------------------------------------------
	// Animation
	//-----------------------------------------------------------------------------------------------------------

	private class AnimationState {
//		public CompareState fromState;
		public CompareState toState;

		public AnimationState(CompareState from, CompareState to) {
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
	private void AnimatePageChange(final CompareState fromState, final CompareState toState) {
		if(mAnimState == null) {
			boolean isTablet = Utils.isTablet(getActivity());

			if(fromState == CompareState.COMPARE && toState == CompareState.DUEL) {
				mAnimState = new AnimationState(fromState, toState);
				mDeviceListBack.setVisibility(View.VISIBLE);
				mDuelListBack.setVisibility(View.VISIBLE);

				if(isTablet) {
					onAnimationEnd(mAnim_PageLeftOut);

				} else {
					mDeviceListBack.startAnimation(mAnim_PageLeftOut);
					mDuelListBack.startAnimation(mAnim_PageRightIn);
				}

			} else if(fromState == CompareState.DUEL && toState == CompareState.COMPARE) {
				mAnimState = new AnimationState(fromState, toState);
				mDeviceListBack.setVisibility(View.VISIBLE);
				mDuelListBack.setVisibility(View.VISIBLE);

				if(isTablet) {
					onAnimationEnd(mAnim_PageRightOut);

				} else {
					mDeviceListBack.startAnimation(mAnim_PageLeftIn);
					mDuelListBack.startAnimation(mAnim_PageRightOut);
				}

			}
		}
	}

	@Override
	public void onAnimationEnd(Animation animation) {
		if((animation == mAnim_PageRightOut ||
				animation == mAnim_PageLeftOut)
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
		if(mState == CompareState.DUEL) {
			AnimatePageChange(mState, CompareState.COMPARE);
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
		return CompareState.COMPARE.name();
	}

	@Override
	public void pageStateChanged() {
		String result_id = "";

		String request = BenchmarkApplication.getAppModel().getFragmentParams(this);
		if(request != null) {
			if(request.contains(", ")) {
				String[] s = request.split(", ");
				result_id = s[1];
			}
		}

		int selectedItemIndex = 0;
		if(!result_id.equals("")) {
			selectedItemIndex = BenchmarkApplication.getModel(getActivity()).getResultSelectorItemPosition(result_id);
		}
		mCurrentTest = (ResultItem)mTestSelectAdapter.getItem(selectedItemIndex);

		mState = CompareState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
		setupState();
		setupText();
		refreshTestSelector();
		refreshCompare();
	}
}
