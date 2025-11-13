/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import net.kishonti.benchui.R;
import net.kishonti.benchui.fragments.BaseFragment;
import net.kishonti.benchui.fragments.CompareFragment;
import net.kishonti.benchui.fragments.HomeFragment;
import net.kishonti.benchui.fragments.InfoFragment;
import net.kishonti.benchui.fragments.ResultsFragment;
import net.kishonti.benchui.fragments.ResultsFragment.ResultsState;
import net.kishonti.benchui.fragments.interfaces.BackButtonFragmentHandler;
import net.kishonti.benchui.fragments.interfaces.HideKeyboardListener;
import net.kishonti.benchui.fragments.interfaces.PageChangeRequestListener;
import net.kishonti.benchui.fragments.interfaces.PageStateChangedHandler;
import net.kishonti.benchui.initialization.InitializationDependentActivity;
import net.kishonti.benchui.model.BenchmarkModel;
import net.kishonti.customcomponents.TabbarBtn;
import net.kishonti.customcomponents.TabbarBtn.OnSelectionChangedListener;
import net.kishonti.swig.Descriptor;
import net.kishonti.swig.SessionVector;
import net.kishonti.theme.Localizator;
import android.app.Fragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.v13.app.FragmentStatePagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.widget.LinearLayout.LayoutParams;

public class MainActivity extends InitializationDependentActivity implements
	OnSelectionChangedListener, OnPageChangeListener, PageChangeRequestListener {

	/**
     * A list that stores the tabs presented by this FragmentPager Activity.
     */
    private ArrayList<Tab> mTabs = new ArrayList<Tab>();

    /**
     * The pager widget, which handles animation and allows swiping horizontally to access previous
     * and next wizard steps.
     */
    private ViewPager mPager;

    /**
     * The pager adapter, which provides the pages to the view pager widget.
     */
    private ScreenSlidePagerAdapter mPagerAdapter;

    /**
     * The LinearLayout that functions as a tabbar.
     */
    private LinearLayout mTabbar;

    private View mRootView;
    public static boolean testAreStarted = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
//		setContentView(R.layout.activity_main);
		setContentView(Utils.getLayoutId(this, "activity_main"));
		mRootView = (View)findViewById(R.id.main_view_back);

		// Log configuration parameters to understand why the screenorientation....
		Configuration c = getResources().getConfiguration();
		Log.i("Configuration", "ScreenHeight: " + c.screenHeightDp);
		Log.i("Configuration", "ScreenWidth: " + c.screenWidthDp);
		Log.i("Configuration", "SmallestWidth: " + c.smallestScreenWidthDp);
		Log.i("Configuration", "ScreenLayout: " + c.screenLayout);
		Log.i("Configuration", "FontScale: " + c.fontScale);


		/**
		 * Set the applications reference of PageChangeRequestListener to this instance.
		 */
		BenchmarkApplication.setPageChangeRequestListener(this);

		/**
		 * Orientation, tablet/phone
		 */
		if(mRootView.getTag() != null && mRootView.getTag().toString().equals("xlarge")) {
			setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	    } else {
	    	setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
	    	overridePendingTransition(0, 0);
	    }

		mTabbar = (LinearLayout) findViewById(R.id.tabbar_back);
        mPager = (ViewPager) findViewById(R.id.pager);
		mPagerAdapter = new ScreenSlidePagerAdapter(getFragmentManager());

		/**
		 * If the tabs are empty fill them. If there are tabs, its OK they only provide
		 * Class instancing methods, and buttons. The references for Fragment instances are
		 * purged on Activity OnDestroy(). See below.
		 */
        if(mTabs.size() == 0) {
    		AddAllTabs();
        }

        /**
		 * WARNING if page count gets too high may cause memory issues.
		 */
        mPager.setAdapter(mPagerAdapter);
        mPager.setOffscreenPageLimit(mPagerAdapter.getCount());
        mPager.setOnPageChangeListener(this);
	}

	/**
	 * If we have display the results from test activity.
	 */
	@Override
	protected void onResume() {
		super.onResume();

		SessionVector sessions = BenchmarkApplication.getModel(this).getSessions();
		String latestSessionId = "";
		if(sessions.size() > 0)
			latestSessionId = "" + sessions.get(0).sessionId();

		boolean isTfwRunning = ((BenchmarkApplication)getApplication()).isTfwActivityRunning();

		if(((BenchmarkApplication)getApplication()).mIsInitialized) {
	    	if (testAreStarted == false && TestSession.closeBrokenSession(getApplicationContext())) {
	    		BenchmarkApplication.getAppModel().SetFragmentParams(ResultsFragment.class.toString(), latestSessionId);
	    		BenchmarkApplication.getAppModel().SetFragmentState(ResultsFragment.class.toString(), ResultsState.RESULT.name());
				ChangePage(1);
			}
		}

		if(testAreStarted) {
    		BenchmarkApplication.getAppModel().SetFragmentParams(ResultsFragment.class.toString(), latestSessionId);
			BenchmarkApplication.getAppModel().SetFragmentState(ResultsFragment.class.toString(), ResultsState.RESULT.name());
			ChangePage(1);
		}

		Intent intent = getIntent();
		String action = intent.getAction();
		if (action != null && action.equals("net.kishonti.benchui.ACTION_SHOW_RESULT")) {
			intent.setAction(null); // show results only once
    		BenchmarkApplication.getAppModel().SetFragmentParams(ResultsFragment.class.toString(), latestSessionId);
			BenchmarkApplication.getAppModel().SetFragmentState(ResultsFragment.class.toString(), ResultsState.RESULT.name());
			ChangePage(1);
		}

		testAreStarted = testAreStarted && isTfwRunning;
	}

	/**
	 * Empty the tabs, delete the reference of us from the application.
	 */
	@Override
	public void onDestroy() {
		BenchmarkApplication.setPageChangeRequestListener(null);
		for (Tab tab : mTabs) {
			tab.mCurrentInstance = null;
		}
		super.onDestroy();
	}


	//-------------------------------------------------------------------------------------------
	/**
     * Handle back button, propagate the call for the active fragment.
     */
	@Override
	public void onBackPressed() {
		Fragment currentFragment = mPagerAdapter.getFragment(mPager.getCurrentItem());
		if(currentFragment instanceof BackButtonFragmentHandler) {
			if (((BackButtonFragmentHandler) currentFragment).HandleBackButton())
				return;
		}
		super.onBackPressed();
	}

    /**
     * The tabbar's on selection changed listener. Notifies the buttons about their correct
     * display style.
     */
	@Override
	public void OnSelectionChanged(TabbarBtn source, boolean selection) {
		if(selection) {
			hideKeyboard();
			int selectedIndex = source.getIndex();
	        mPager.setCurrentItem(selectedIndex);
	        for (Tab tab : mTabs) {
				if(tab.getIndex() != selectedIndex) {
					tab.getBtn().setSelected(false, false);
				}
			}
		}
	}

	/**
     * Adds a new tab to the presentable tabs. Notifies the Pager about the new tab.
     */
	protected void AddTab(String name, Class<? extends BaseFragment> f, int pictureId, int selectedPictureId) {
		int index = mTabs.size();
		mTabs.add(new Tab(index, name, f, pictureId, selectedPictureId, this));
		if(mPagerAdapter != null)
			mPagerAdapter.notifyDataSetChanged();
	}

	/**
	 * Adds all the tab to the tabbar, and all fragments to the pager.
	 * Override this method to have you own tabs represented
	 */
	protected void AddAllTabs() {
		AddTab(Localizator.getString(this, "TabHome"), HomeFragment.class, R.drawable.app_home_icon, R.drawable.app_home_icon_sel);
        AddTab(Localizator.getString(this, "TabResults"), ResultsFragment.class, R.drawable.results_icon, R.drawable.results_icon_sel);
        AddTab(Localizator.getString(this, "TabCompare"), CompareFragment.class, R.drawable.compare_icon, R.drawable.compare_icon_sel);
        AddTab(Localizator.getString(this, "TabInfo"), InfoFragment.class, R.drawable.info_icon, R.drawable.info_icon_sel);
	}


	/**
	 * Tries to hide the keyboard if the active fragment has an interface to do so.
	 */
	private void hideKeyboard() {
		Fragment currentFragment = mPagerAdapter.getFragment(mPager.getCurrentItem());
		if(currentFragment instanceof HideKeyboardListener) {
			((HideKeyboardListener) currentFragment).hideKeyboard();
		}
	}

	//----------------------------------------------------------------------------------------------
	/**
     * OnPageListener methods
     */

	/**
	 * If scrolling occured hide the keyboard.
	 */
	@Override
	public void onPageScrollStateChanged(int arg0) {
		hideKeyboard();
	}

	/**
	 * Pager's selection changed listener. Updates the buttons visibility in
	 * the tabbar.
	 */
	@Override
	public void onPageSelected(int postition) {
		int selectedIndex = postition;
        mPager.setCurrentItem(selectedIndex);
        for (Tab tab : mTabs) {
			if(tab.getIndex() != selectedIndex) {
				tab.getBtn().setSelected(false, false);
			} else {
				tab.getBtn().setSelected(true, false);
			}
		}
	}

	/**
	 * Changes the current page of the mPager from code. Notifies
	 * the activated page that its now the displayed page.
	 */
	@Override
	public void ChangePage(int pageIndex) {
		onPageSelected(pageIndex);

		Fragment currentFragment = mPagerAdapter.getFragment(pageIndex);
		if(currentFragment instanceof PageStateChangedHandler) {
			((PageStateChangedHandler) currentFragment).pageStateChanged();
		}
	}

	/**
	 * Needs implementation. We do not handle the animations state so no real thing here.
	 */
	@Override
	public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
	}

	//----------------------------------------------------------------------------------------------

	/**
	 * Starts the tests based on the list provided.
	 * @param selectedTests List of the tests to run.
	 */
	public void startTestSession(List<String> selectedTests) {
		if (selectedTests.size() == 0)
			return;

		String[] tests = new String[selectedTests.size()];
		tests = selectedTests.toArray(tests);
    	DescriptorFactory factory = ((BenchmarkApplication)BenchmarkApplication.instance).getDescriptorFactory();


		ArrayList<Descriptor> descriptors = new ArrayList<Descriptor>();

		for (String testId : selectedTests) {

			Bundle args = new Bundle();
			SharedPreferences prefs = getSharedPreferences("prefs", Context.MODE_PRIVATE);
			if(prefs.getBoolean("forceBrightness", false)) {
				float brightness = prefs.getInt("brightness", 255) / 255.0f;
				args.putFloat("brightness", brightness);
			}
			if(prefs.getBoolean("forceResolution", false)) {
				args.putInt("-width", prefs.getInt("customOnscreenWidth", 200));
				args.putInt("-height", prefs.getInt("customOnscreenHeight", 200));
			}
			args.putBoolean("interop", prefs.getBoolean("interop", true));
			int configIndex = prefs.getInt("selectedOpenclConfigIndex", 0);
			args.putInt("configIndex", configIndex);

			Descriptor test = factory.getDescriptorForId(testId, args);
			if (testId.contains("composite")){
				test.setTestId(testId);
			}

			if(test!=null)
				descriptors.add(test);
			Log.v("BACKGROUND","Descriptor!");
		}

		if(descriptors.size() == 0) {
	    	Toast.makeText(getApplicationContext(), "Failed to start tests.", Toast.LENGTH_LONG).show();
    	 } else {
    		try {
    			testAreStarted = true;
				TestSession.start(getApplicationContext(), descriptors,false);
			} catch (IOException e) {
				Toast.makeText(getApplicationContext(), "Failed to start test: " + e.getLocalizedMessage(), Toast.LENGTH_LONG).show();
			}
    	 }
	}

	//----------------------------------------------------------------------------------------------

	/**
	 * Handle motion events and controller key presses
	 */
//	@Override
//	public boolean dispatchGenericMotionEvent(MotionEvent ev) {
//
//
//		return super.dispatchGenericMotionEvent(ev);
//	}
//
	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		boolean handled = false;

		int currentPage = mPager.getCurrentItem();
		handled = super.dispatchKeyEvent(event);

		if(!handled && currentPage == mPager.getCurrentItem() &&
				(event.getKeyCode() == KeyEvent.KEYCODE_DPAD_LEFT ||
				event.getKeyCode() == KeyEvent.KEYCODE_DPAD_RIGHT)) {
			handled = true;

			if(event.getKeyCode() == KeyEvent.KEYCODE_DPAD_LEFT && event.getAction() != KeyEvent.ACTION_UP) {
				currentPage = mPager.getCurrentItem();
				int pageCount = mPager.getChildCount();
				int prevPage = (currentPage + pageCount - 1)%pageCount;
				mPager.setCurrentItem(prevPage);
			}
			else if(event.getKeyCode() == KeyEvent.KEYCODE_DPAD_RIGHT && event.getAction() != KeyEvent.ACTION_UP) {
				currentPage = mPager.getCurrentItem();
				int pageCount = mPager.getChildCount();
				int prevPage = (currentPage + pageCount + 1)%pageCount;
				mPager.setCurrentItem(prevPage);
			}
		}

//		Log.i("Focus", "Focus is on: " + getCurrentFocus());
		return handled;
	}

	//-------------------------------------------------------------------------------------------

	/**
	 * ********************************************************************************
	 * Tab:
	 *
     * Inner class that encapsulates a Fragment tab and its button in the tabbar.
     * *********************************************************************************
     */
	private class Tab {
		private int mIndex;
		private Class<? extends BaseFragment> mFragment;
		private TabbarBtn mButton;
		public BaseFragment mCurrentInstance = null;

		/**
		 * Creates a tab instance.
		 * @param index Index of the tab in the tabbar.
		 * @param name Display name's StringID of the tabbar button.
		 * @param fragment Class of the fragment associated with this tab.
		 * @param pictureId Default picture id of the tabbar button.
		 * @param selectedPictureId Selected picture id of the tabbar button.
		 * @param activity Activity to use when creating layouts, getting resource ids (MainActivity now).
		 */
		public Tab(int index, String name, Class<? extends BaseFragment> fragment, int pictureId, int selectedPictureId, final MainActivity activity) {
			mIndex = index;
			mFragment = fragment;

			int btnPos = TabbarBtn.BTNPOS_ONLY;
			if(index == 0) {
				btnPos = TabbarBtn.BTNPOS_ONLY;
			} else {
				mTabs.get(index - 1).getBtn().setBtnPos(index == 1 ? TabbarBtn.BTNPOS_LEFTEND : TabbarBtn.BTNPOS_CENTER);
				btnPos = TabbarBtn.BTNPOS_RIGHTEND;
			}

			mButton = new TabbarBtn(activity, name, pictureId, selectedPictureId, btnPos, index == 0, index);
			LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(0, LayoutParams.MATCH_PARENT, 1.0f);
			mButton.setLayoutParams(params);
			mButton.setOnSelectionChangedListener(activity);

			if(mTabbar != null) {
				mTabbar.addView(mButton);
			}
		}

		/**
		 * Gets the index of the tab.
		 * @return Integer index of the tab.
		 */
		public int getIndex() {
			return mIndex;
		}

		/**
		 * Creates an instance of the Fragment associated with this tab.
		 * We shouldn't store reference to the created instance here, coz the
		 * restored instances won't be created here.
		 * @return Created fragment instance.
		 */
		public Fragment getFragmentInstance() {
			try {
				BaseFragment f = mFragment.newInstance();
				BenchmarkModel appModel = BenchmarkApplication.getAppModel();
				appModel.SetFragmentState(f, f.getDefaultStateString());

				return f;

			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}
			 return null;
		}

		/**
		 * Returns the Button residing in the tabbar.
		 * @return TabbarBtn instance.
		 */
		public TabbarBtn getBtn() {
			return mButton;
		}
	}

	/**
	 * ********************************************************************************
	 * ScreenSlidePagerAdapter:
	 *
     * A simple pager adapter that represents 5 ScreenSlidePageFragment objects, in
     * sequence.
     * *********************************************************************************
     */
    private class ScreenSlidePagerAdapter extends FragmentStatePagerAdapter {

        public ScreenSlidePagerAdapter(FragmentManager fm) {
            super(fm);
        }

        /**
         * Creates the fragment as presented in the base class but saves a reference for it
         * in the main activity's tab collection for page changing actions.
         * This method is always called (even on restoring operations).
         */
		@Override
		public Object instantiateItem(ViewGroup container, int position) {
			Fragment f = (Fragment)super.instantiateItem(container, position);
			if(f instanceof BaseFragment)
				mTabs.get(position).mCurrentInstance = (BaseFragment)f;
			else
				mTabs.get(position).mCurrentInstance = null;

			return f;
		}

		/**
		 * Returns the saved fragment instance for the desired position.
		 * @param position Desired fragments position in the tabs.
		 * @return Fragment instance.
		 */
		public Fragment getFragment(int position) {
			return mTabs.get(position).mCurrentInstance;
		}

		/**
		 * Used only by the instantiation process.
		 * ATTENTION!!!
		 * This is NOT called on state restoration!!!!!
		 * The restored fragments will not have any connection with the Tabs
		 * should it be that just this method connects the two.
		 */
        @Override
        public Fragment getItem(int position) {
        	return mTabs.get(position).getFragmentInstance();
        }

        /**
         * Returns the count of the stored fragments
         */
        @Override
        public int getCount() {
            return mTabs.size();
        }
    }

}
