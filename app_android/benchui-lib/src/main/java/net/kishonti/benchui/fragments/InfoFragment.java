/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Animation.AnimationListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.fragments.HomeFragment.HomeState;
import net.kishonti.benchui.lists.adapters.DeviceInfoAdapter;
import net.kishonti.benchui.lists.adapters.GLDetailAdapter;
import net.kishonti.benchui.lists.adapters.DeviceInfoAdapter.InfoItem;
import net.kishonti.systeminfo.swig.ApiPlatformVector;
import net.kishonti.systeminfo.swig.DataFormatter;
import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.systeminfo.swig.systeminfolib;
import net.kishonti.theme.Localizator;

public class InfoFragment extends BaseFragment implements 
	OnClickListener, OnItemClickListener, AnimationListener {

	/**
	 * Root views and backgrounds.
	 */
	private View mRootView;
	
	/**
	 * Navigation
	 */
	private Button mNavbarLeft;
	private Button mNavbarRight;
	private TextView mNavbarTitle;
	
	/**
	 * Main controls
	 */
	private ListView mInfoList;
	private ListView mDetailList;
	
	/**
	 * Animation
	 */
	public enum InfoState {
		NORMAL, DETAIL
	}
	private InfoState mState = InfoState.NORMAL;
	private Animation mAnim_PageLeftIn;
	private Animation mAnim_PageLeftOut;
	private Animation mAnim_PageRightIn;
	private Animation mAnim_PageRightOut;
	private AnimationState mAnimState;
	
	DeviceInfoAdapter mInfoAdapter;
	GLDetailAdapter mDetailAdapter;
	private BroadcastReceiver mBatteryBrouadcastReciever;
	
	private String DetailTitle = "TabGlInfo";
	

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
//		mRootView = inflater.inflate(R.layout.fragment_info, container, false);
		mRootView = inflater.inflate(Utils.getLayoutId(getActivity(), "fragment_info"), container, false);

		BenchmarkApplication.getAppModel().SetFragmentState(this, mState.name());
		
		mNavbarLeft = (Button)mRootView.findViewById(R.id.headerButtonLeft);
		mNavbarRight = (Button)mRootView.findViewById(R.id.headerButtonRight);
		mNavbarTitle = (TextView)mRootView.findViewById(R.id.headerTitle);
		
		mInfoList = (ListView)mRootView.findViewById(R.id.info_list);
		mDetailList = (ListView)mRootView.findViewById(R.id.info_detailList);
		
		// Setup the button handlers
		mNavbarLeft.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		
		mInfoList.setOnItemClickListener(this);
		
		// Notification handler for battery changes.
		mBatteryBrouadcastReciever = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				if(mInfoAdapter != null) {
					double level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
					double scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0);
					level /= scale;
					boolean plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0) > 0;
					boolean charging = intent.getIntExtra(BatteryManager.EXTRA_STATUS, 0) == BatteryManager.BATTERY_STATUS_CHARGING;
					
					mInfoAdapter.refreshBattery(level, charging, plugged);
					mInfoAdapter.notifyDataSetChanged();
				}
			}
		};

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
		
		mState = InfoState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
		
		if(((BenchmarkApplication)getActivity().getApplication()).mIsInitialized) {
			Properties props = ((BenchmarkApplication) getActivity().getApplication()).getProperties();
			mInfoAdapter = new DeviceInfoAdapter(getActivity(), props);
			mInfoList.setAdapter(mInfoAdapter);
			mInfoAdapter.notifyDataSetChanged();
		}
		
		getActivity().registerReceiver(
				mBatteryBrouadcastReciever, 
				new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		setupState();
		setupText();
	}
	
	@Override
	public void onPause() {
		mInfoAdapter = null;
		mInfoList.setAdapter(null);
		
		getActivity().unregisterReceiver(mBatteryBrouadcastReciever);
		super.onPause();
	}
	
	/**
	 * Manages the state change visibility.
	 */
	private void setupState() {
		
		if(mState == InfoState.NORMAL) {
			mInfoList.setVisibility(View.VISIBLE);
			mDetailList.setVisibility(View.GONE);
			mNavbarLeft.setVisibility(View.GONE);
			mNavbarRight.setVisibility(View.INVISIBLE);
			
		} else if(mState == InfoState.DETAIL) {
			mInfoList.setVisibility(View.GONE);
			mDetailList.setVisibility(View.VISIBLE);
			mNavbarLeft.setVisibility(View.VISIBLE);
			mNavbarRight.setVisibility(View.INVISIBLE);
			
		}
	}
	
	/**
	 * Setup state dependent texts
	 */
	private void setupText() {
		if(mState == InfoState.NORMAL) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TabInfo"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			
		} else if(mState == InfoState.DETAIL) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), DetailTitle));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));
			
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
			if(mState == InfoState.DETAIL) {
				AnimatePageChange(mState, InfoState.NORMAL);
			}
		}
	}

	/**
	 * Handler for item clicks in lists
	 */
	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		if (parent.getId() == R.id.info_list) {
			InfoItem item = (InfoItem) mInfoList.getItemAtPosition(position);
			if(item.mTitle.contains("GL")) {
				
				Properties props = ((BenchmarkApplication) getActivity().getApplication()).getProperties();
				DataFormatter df = new DataFormatter();
				df.SetProperties(props);
				ApiPlatformVector apv = df.GetApiStream("api/gles");
				mDetailAdapter = new GLDetailAdapter(getActivity(), apv);
				mDetailList.setAdapter(mDetailAdapter);
				mDetailAdapter.notifyDataSetChanged();
				DetailTitle = "TabDetailedInfo";
				
				AnimatePageChange(mState, InfoState.DETAIL);

			} else if(item.mTitle.contains("CL")) {	
				Properties props = ((BenchmarkApplication) getActivity().getApplication()).getProperties();
				DataFormatter df = new DataFormatter();
				df.SetProperties(props);
				ApiPlatformVector apv = df.GetApiStream("api/cl");
				mDetailAdapter = new GLDetailAdapter(getActivity(), apv);
				mDetailList.setAdapter(mDetailAdapter);
				mDetailAdapter.notifyDataSetChanged();
				DetailTitle = "TabDetailedInfo";
				
				AnimatePageChange(mState, InfoState.DETAIL);

			} else if(item.mTitle.contains("CUDA")) {	
				Properties props = ((BenchmarkApplication) getActivity().getApplication()).getProperties();
				DataFormatter df = new DataFormatter();
				df.SetProperties(props);
				ApiPlatformVector apv = df.GetApiStream("api/cuda");
				mDetailAdapter = new GLDetailAdapter(getActivity(), apv);
				mDetailList.setAdapter(mDetailAdapter);
				mDetailAdapter.notifyDataSetChanged();
				DetailTitle = "TabDetailedInfo";
				
				AnimatePageChange(mState, InfoState.DETAIL);
			} else if (item.mTitle.contains("VULKAN")){
				Properties props = ((BenchmarkApplication) getActivity().getApplication()).getProperties();
				DataFormatter df = new DataFormatter();
				df.SetProperties(props);
				ApiPlatformVector apv = df.GetApiStream("api/vulkan");
				mDetailAdapter = new GLDetailAdapter(getActivity(), apv);
				mDetailList.setAdapter(mDetailAdapter);
				mDetailAdapter.notifyDataSetChanged();
				DetailTitle = "TabDetailedInfo";
				AnimatePageChange(mState, InfoState.DETAIL);
			}
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------------------
	// Animation
	//-----------------------------------------------------------------------------------------------------------
	
	private class AnimationState {
//		public InfoState fromState;
		public InfoState toState;
		
		public AnimationState(InfoState from, InfoState to) {
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
	private void AnimatePageChange(final InfoState fromState, final InfoState toState) {
		if(mAnimState == null) {
			
			if(fromState == InfoState.NORMAL && toState == InfoState.DETAIL) {
				mAnimState = new AnimationState(fromState, toState);
				mInfoList.setVisibility(View.VISIBLE);
				mDetailList.setVisibility(View.VISIBLE);

				mInfoList.startAnimation(mAnim_PageLeftOut);
				mDetailList.startAnimation(mAnim_PageRightIn);
				
			} else if(fromState == InfoState.DETAIL && toState == InfoState.NORMAL) {
				mAnimState = new AnimationState(fromState, toState);
				mInfoList.setVisibility(View.VISIBLE);
				mDetailList.setVisibility(View.VISIBLE);

				mInfoList.startAnimation(mAnim_PageLeftIn);
				mDetailList.startAnimation(mAnim_PageRightOut);
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
		if(mState == InfoState.DETAIL) {
			AnimatePageChange(mState, InfoState.NORMAL);
			handled = true;
		
		} else if(getPageChangeRequestListener() != null) {
			BenchmarkApplication.getAppModel().SetFragmentState(HomeFragment.class.toString(), HomeState.NORMAL.name());
			getPageChangeRequestListener().ChangePage(0);
			handled = true;
		}
		
		return handled;
	}

	@Override
	public void pageStateChanged() {
		mState = InfoState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
	}

	@Override
	public String getDefaultStateString() {
		return InfoState.NORMAL.name();
	}
}
