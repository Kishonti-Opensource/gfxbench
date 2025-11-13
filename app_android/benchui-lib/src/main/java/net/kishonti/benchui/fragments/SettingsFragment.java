/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import java.io.File;
import java.util.HashSet;

import org.apache.commons.io.FileUtils;

import android.app.Activity;
import android.app.DialogFragment;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Animation.AnimationListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.TextView.OnEditorActionListener;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.dialogs.ClearResultDialog;
import net.kishonti.benchui.dialogs.DeleteUserDialog;
import net.kishonti.benchui.dialogs.CommonDialog;
import net.kishonti.benchui.dialogs.LicenseDialog;
import net.kishonti.benchui.dialogs.CommonDialog.OnDialogHandler;
import net.kishonti.benchui.fragments.HomeFragment.HomeState;
import net.kishonti.benchui.fragments.interfaces.HideKeyboardListener;
import net.kishonti.benchui.initialization.DetermineBigDataDirTask;
import net.kishonti.benchui.initialization.EULATask;
import net.kishonti.benchui.initialization.InitializerApplication;

import net.kishonti.customcomponents.FlipSwitch;
import net.kishonti.customcomponents.FlipSwitch.OnStateChangeListener;
import net.kishonti.theme.Localizator;

public class SettingsFragment extends BaseFragment implements
	OnClickListener, OnStateChangeListener, OnEditorActionListener,
	OnDialogHandler, HideKeyboardListener, AnimationListener {

	/**
	 * Root views and backgrounds.
	 */
	protected View mRootView;
	protected View mSettingsBack;
	protected View mRegisterBack;
	protected View mFollowUsBack;

	/**
	 * Navigation
	 */
	protected Button mNavbarLeft;
	protected Button mNavbarRight;
	protected TextView mNavbarTitle;

	/**
	 * Main controls
	 */
	protected LinearLayout mPartStore;
	protected View mCommonBack;
	protected TextView mWhatIsThisTitle;
	protected TextView mWhatIsThis;
	protected Button mFollowUsBtn;
	protected Button mReadLicenseBtn;
	protected Button mClearCacheBtn;
	protected TextView mDisplayDesktopTV;
	protected FlipSwitch mDisplayDesktop;

	/**
	 * User specific
	 */
	protected View mUserSettingsBack;
	protected TextView mUserStatus;
	protected EditText mUserName;
	protected EditText mPass;
	protected TextView mWelcome;
	protected TextView mWelcomeUser;
	protected Button mLogin;
	protected Button mRegister;
	protected Button mDeleteUser;

	/**
	 * Follow us
	 */
	protected ImageView mFollow_Logo;
	protected TextView mFollow_Info;
	protected ImageButton mFollow_Twitter;
	protected ImageButton mFollow_Youtube;
	protected ImageButton mFollow_LinkedIn;
	protected ImageButton mFollow_Vimeo;
	protected ImageButton mFollow_Facebook;
	protected ImageButton mFollow_GooglePlus;
	protected Button mFollow_Website;
	protected Button mFollow_Kishonti;
	protected Button mFollow_ContactUs;

	/**
	 * Register
	 */
	protected TextView mRegister_MainText;
	protected EditText mRegister_User;
	protected EditText mRegister_Pass;
	protected EditText mRegister_PassAgain;
	protected EditText mRegister_Email;
	protected Button mRegister_ConfirmButton;

	/**
	 * Corporate features
	 */
	protected TextView mCorporate_Features;
	protected TextView mCorporate_CommercialTitle;
	protected TextView mCorporate_Commercial;
	protected TextView mCorporate_OnResText;
	protected FlipSwitch mCorporate_OnResSwitch;
	protected TextView mCorporate_WidthText;
	protected EditText mCorporate_Width;
	protected TextView mCorporate_HeightText;
	protected EditText mCorporate_Height;
	protected TextView mCorporate_ForceBrightness;
	protected FlipSwitch mCorporate_ForceBrightnessSwitch;
	protected ToggleButton mCorporate_Brightness0;
	protected ToggleButton mCorporate_Brightness25;
	protected ToggleButton mCorporate_Brightness50;
	protected ToggleButton mCorporate_Brightness75;
	protected ToggleButton mCorporate_Brightness100;

	/**
	 * Animation
	 */
	public enum SettingsState {
		NORMAL, REGISTER, FOLLOW
	}
	protected SettingsState mState = SettingsState.NORMAL;
	private Animation mAnim_PageLeftIn;
	private Animation mAnim_PageLeftOut;
	private Animation mAnim_PageRightIn;
	private Animation mAnim_PageRightOut;
	private AnimationState mAnimState;

	protected boolean mCorporate = true;
	protected boolean mForceBrightness = false;
	protected boolean mForceResolution = false;
	protected boolean mShowDesktop = false;
	protected float mBrightness = 0.5f;
	protected int mWidth = 100;
	protected int mHeight = 100;
	private final LicenseDialog licenseDialog = new LicenseDialog();

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
//		mRootView = inflater.inflate(R.layout.fragment_settings, container, false);
		mRootView = inflater.inflate(Utils.getLayoutId(getActivity(), "fragment_settings"), container, false);

		BenchmarkApplication.getAppModel().SetFragmentState(this, mState.name());

		mSettingsBack = (View)mRootView.findViewById(R.id.settings_scrollBack);
		mRegisterBack = (View)mRootView.findViewById(R.id.settings_registerBack);
		mFollowUsBack = (View)mRootView.findViewById(R.id.settings_followusBack);

		mNavbarLeft = (Button)mRootView.findViewById(R.id.headerButtonLeft);
		mNavbarRight = (Button)mRootView.findViewById(R.id.headerButtonRight);
		mNavbarTitle = (TextView)mRootView.findViewById(R.id.headerTitle);
//		mNavbarIcon = (ImageView)mRootView.findViewById(R.id.headerIcon);

		mPartStore = (LinearLayout)mRootView.findViewById(R.id.settings_partStore);
		mCommonBack = (View)mRootView.findViewById(R.id.settings_commonBack);
		mWhatIsThisTitle = (TextView)mRootView.findViewById(R.id.settings_whatIsThis);
		mWhatIsThis = (TextView)mRootView.findViewById(R.id.settings_whatIsThisContent);
		mFollowUsBtn = (Button)mRootView.findViewById(R.id.settings_followUs);
		mReadLicenseBtn = (Button)mRootView.findViewById(R.id.settings_readLicense);
		mClearCacheBtn = (Button)mRootView.findViewById(R.id.settings_clearCache);
		mDisplayDesktop = (FlipSwitch)mRootView.findViewById(R.id.settings_DisplayDesktop);
		mDisplayDesktopTV = (TextView)mRootView.findViewById(R.id.settings_textViewDisplayDesktop);

		mUserSettingsBack = (View)mRootView.findViewById(R.id.settings_userBack);
		mUserStatus = (TextView)mRootView.findViewById(R.id.settings_userStatusMessage);
		mUserName = (EditText)mRootView.findViewById(R.id.settings_editableUserName);
		mPass = (EditText)mRootView.findViewById(R.id.settings_editableUserPassword);
		mWelcome = (TextView)mRootView.findViewById(R.id.settings_userWelcome);
		mWelcomeUser = (TextView)mRootView.findViewById(R.id.settings_username);
		mLogin = (Button)mRootView.findViewById(R.id.settings_login);
		mRegister = (Button)mRootView.findViewById(R.id.settings_register);
		mDeleteUser = (Button)mRootView.findViewById(R.id.settings_delete_user);

		mFollow_Logo = (ImageView)mRootView.findViewById(R.id.followus_logo);
		mFollow_Info = (TextView)mRootView.findViewById(R.id.followus_info);
		mFollow_Twitter = (ImageButton)mRootView.findViewById(R.id.followus_twitter);
		mFollow_Youtube = (ImageButton)mRootView.findViewById(R.id.followus_youtube);
		mFollow_LinkedIn = (ImageButton)mRootView.findViewById(R.id.followus_linkedin);
		mFollow_Vimeo = (ImageButton)mRootView.findViewById(R.id.followus_vimeo);
		mFollow_Facebook = (ImageButton)mRootView.findViewById(R.id.followus_facebook);
		mFollow_GooglePlus = (ImageButton)mRootView.findViewById(R.id.followus_googleplus);
		mFollow_Website = (Button)mRootView.findViewById(R.id.followus_website);
		mFollow_Kishonti = (Button)mRootView.findViewById(R.id.followus_kishonti);
		mFollow_ContactUs = (Button)mRootView.findViewById(R.id.followus_contactUs);

		mRegister_MainText = (TextView)mRootView.findViewById(R.id.register_mainText);
		mRegister_User = (EditText)mRootView.findViewById(R.id.register_name);
		mRegister_Pass = (EditText)mRootView.findViewById(R.id.register_password);
		mRegister_PassAgain = (EditText)mRootView.findViewById(R.id.register_repassword);
		mRegister_Email = (EditText)mRootView.findViewById(R.id.register_email);
		mRegister_ConfirmButton = (Button)mRootView.findViewById(R.id.register_registerBtn);

		mCorporate_Features = (TextView)mRootView.findViewById(R.id.settings_textViewCorporateFeatures);
		mCorporate_CommercialTitle = (TextView)mRootView.findViewById(R.id.settings_textViewCorporateCommercialTitle);
		mCorporate_Commercial = (TextView)mRootView.findViewById(R.id.settings_textViewCorporateCommercial);
		mCorporate_OnResText = (TextView)mRootView.findViewById(R.id.settings_textViewSetOnscrResolution);
		mCorporate_OnResSwitch = (FlipSwitch)mRootView.findViewById(R.id.settings_onscreen);
		mCorporate_WidthText = (TextView)mRootView.findViewById(R.id.settings_tvWidth);
		mCorporate_Width = (EditText)mRootView.findViewById(R.id.settings_onscreenWidth);
		mCorporate_HeightText = (TextView)mRootView.findViewById(R.id.settings_tvHeight);
		mCorporate_Height = (EditText)mRootView.findViewById(R.id.settings_onscreenHeight);
		mCorporate_ForceBrightness = (TextView)mRootView.findViewById(R.id.settings_textViewForceDisplayBrightness);
		mCorporate_ForceBrightnessSwitch = (FlipSwitch)mRootView.findViewById(R.id.settings_forceDisplayBrightness);
		mCorporate_Brightness0 = (ToggleButton)mRootView.findViewById(R.id.settings_btBrightness0);
		mCorporate_Brightness25 = (ToggleButton)mRootView.findViewById(R.id.settings_btBrightness25);
		mCorporate_Brightness50 = (ToggleButton)mRootView.findViewById(R.id.settings_btBrightness50);
		mCorporate_Brightness75 = (ToggleButton)mRootView.findViewById(R.id.settings_btBrightness75);
		mCorporate_Brightness100 = (ToggleButton)mRootView.findViewById(R.id.settings_btBrightness100);

		// Setup the button handlers
		mNavbarLeft.setOnClickListener(this);
		mNavbarRight.setOnClickListener(this);
		mFollowUsBtn.setOnClickListener(this);
		mReadLicenseBtn.setOnClickListener(this);
		mClearCacheBtn.setOnClickListener(this);
		mLogin.setOnClickListener(this);
		mRegister.setOnClickListener(this);
		mDeleteUser.setOnClickListener(this);
		mFollow_Twitter.setOnClickListener(this);
		mFollow_Youtube.setOnClickListener(this);
		mFollow_LinkedIn.setOnClickListener(this);
		mFollow_Vimeo.setOnClickListener(this);
		mFollow_Facebook.setOnClickListener(this);
		mFollow_GooglePlus.setOnClickListener(this);
		mFollow_Website.setOnClickListener(this);
		mFollow_Kishonti.setOnClickListener(this);
		mFollow_ContactUs.setOnClickListener(this);
		mRegister_ConfirmButton.setOnClickListener(this);
		mCorporate_OnResSwitch.setOnStateChangeListener(this);
		mCorporate_ForceBrightnessSwitch.setOnStateChangeListener(this);
		mCorporate_Brightness0.setOnClickListener(this);
		mCorporate_Brightness25.setOnClickListener(this);
		mCorporate_Brightness50.setOnClickListener(this);
		mCorporate_Brightness75.setOnClickListener(this);
		mCorporate_Brightness100.setOnClickListener(this);
		mDisplayDesktop.setOnStateChangeListener(this);

		View.OnFocusChangeListener saveNewResolutions = new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				int value = -1;
				try {
					value = Integer.parseInt(((EditText) v).getText().toString());
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}

				if (hasFocus == false) {
					if (v.getId() == R.id.settings_onscreenWidth) {
						getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenWidth", value).commit();
						mWidth = value;
					} else if (v.getId() == R.id.settings_onscreenHeight) {
						getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenHeight", value).commit();
						mHeight = value;
					}
				} else {
					if (v.getId() == R.id.settings_onscreenWidth) {
						getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenWidth", value).commit();
						mWidth = value;
						((EditText) v).setImeOptions(EditorInfo.IME_ACTION_DONE);
					} else if (v.getId() == R.id.settings_onscreenHeight) {
						getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenHeight", value).commit();
						mHeight = value;
						((EditText) v).setImeOptions(EditorInfo.IME_ACTION_DONE);
					}
				}
			}
		};

		mCorporate_Width.setOnFocusChangeListener(saveNewResolutions);
		mCorporate_Height.setOnFocusChangeListener(saveNewResolutions);

		mCorporate_Width.setOnEditorActionListener(this);
		mCorporate_Height.setOnEditorActionListener(this);

	    mAnim_PageRightIn = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_right_in);
	    mAnim_PageRightOut = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_right_out);
	    mAnim_PageLeftIn = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_left_in);
	    mAnim_PageLeftOut = AnimationUtils.loadAnimation(getActivity(), R.anim.slide_left_out);

	    mAnim_PageRightIn.setAnimationListener(this);
	    mAnim_PageRightOut.setAnimationListener(this);
	    mAnim_PageLeftIn.setAnimationListener(this);
	    mAnim_PageLeftOut.setAnimationListener(this);

		setupText();
		setupFocus();


		//TODO move this to the build phase instead... (it's not easy due to the current no src in the mainapp approach)
		if(getActivity().getResources().getString(R.string.app_product_id).contains("compubench")) {
			addCorporateSwitch("Interop", "interop", true);
			mDisplayDesktop.changeState(mShowDesktop);
			mDisplayDesktop.setEnabled(false);
		}

		return mRootView;
	}

	protected void addCorporateSwitch(String name, final String connectorName, boolean state) {
		LinearLayout mCorporateBack = (LinearLayout)mRootView.findViewById(R.id.settings_corporateBack);

		// Back LinearLayout for the views
		LinearLayout AdditionalSwitchBackground = new LinearLayout(getActivity());
		AdditionalSwitchBackground.setGravity(Gravity.CENTER);
		AdditionalSwitchBackground.setOrientation(LinearLayout.HORIZONTAL);
		LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		lp.setMargins(0, (int)getResources().getDimension(R.dimen.view_margin), 0, (int)getResources().getDimension(R.dimen.view_margin));

		// Textview to present the name
		TextView AdditionalSwitchName = new TextView(getActivity());
		AdditionalSwitchName.setTextColor(getActivity().getResources().getColor(R.color.enable_disable));
		AdditionalSwitchName.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
		AdditionalSwitchName.setText(name);
		LinearLayout.LayoutParams lpN = new LinearLayout.LayoutParams(0, LayoutParams.MATCH_PARENT, 0.5f);

		// Switch to use for the boolean set
		FlipSwitch AdditionalSwitchSwitch = new FlipSwitch(getActivity());
		AdditionalSwitchSwitch.changeState(true);
		LinearLayout.LayoutParams lpS = new LinearLayout.LayoutParams(0, LayoutParams.MATCH_PARENT, 0.5f);
		lpS.gravity = Gravity.RIGHT;

		// Default state
		SharedPreferences prefs = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE);
		boolean currentState = prefs.getBoolean(connectorName, state);
		prefs.edit().putBoolean(connectorName, currentState).commit();
		AdditionalSwitchSwitch.changeState(currentState);

		// State change handling
		AdditionalSwitchSwitch.setOnStateChangeListener(new OnStateChangeListener() {

			@Override
			public void onStateChanged(View view, boolean state) {
				getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE)
					.edit().putBoolean(connectorName, state).commit();
			}
		});

		AdditionalSwitchBackground.addView(AdditionalSwitchName, 0, lpN);
		AdditionalSwitchBackground.addView(AdditionalSwitchSwitch, 1, lpS);

		// insert into main view
		mCorporateBack.addView(AdditionalSwitchBackground, 2, lp);
	}

	private void setupFocus() {
		mPass.setNextFocusUpId(R.id.settings_editableUserName);
		mPass.setNextFocusLeftId(R.id.settings_editableUserName);
		mUserName.setNextFocusDownId(R.id.settings_editableUserPassword);
		mUserName.setNextFocusRightId(R.id.settings_editableUserPassword);
		mUserName.setNextFocusForwardId(R.id.settings_editableUserPassword);

		mPass.setNextFocusDownId(R.id.settings_login);
		mPass.setNextFocusRightId(R.id.settings_login);
		mPass.setNextFocusForwardId(R.id.settings_login);
		mPass.setNextFocusUpId(R.id.settings_editableUserName);
		mPass.setNextFocusLeftId(R.id.settings_editableUserName);

		mLogin.setNextFocusDownId(R.id.settings_register);
		mLogin.setNextFocusRightId(R.id.settings_register);
		mLogin.setNextFocusForwardId(R.id.settings_register);
		mLogin.setNextFocusUpId(R.id.settings_editableUserPassword);
		mLogin.setNextFocusLeftId(R.id.settings_editableUserPassword);

		mRegister.setNextFocusDownId(R.id.settings_followUs);
		mRegister.setNextFocusRightId(R.id.settings_followUs);
		mRegister.setNextFocusForwardId(R.id.settings_followUs);
		mRegister.setNextFocusUpId(R.id.settings_login);
		mRegister.setNextFocusLeftId(R.id.settings_followUs);

		mFollowUsBtn.setNextFocusDownId(R.id.settings_readLicense);
		mFollowUsBtn.setNextFocusRightId(R.id.settings_readLicense);
		mFollowUsBtn.setNextFocusForwardId(R.id.settings_readLicense);
		mFollowUsBtn.setNextFocusUpId(R.id.settings_register);
		mFollowUsBtn.setNextFocusLeftId(R.id.settings_register);

		mReadLicenseBtn.setNextFocusDownId(R.id.settings_clearCache);
		mReadLicenseBtn.setNextFocusRightId(R.id.settings_clearCache);
		mReadLicenseBtn.setNextFocusForwardId(R.id.settings_clearCache);
		mReadLicenseBtn.setNextFocusUpId(R.id.settings_followUs);
		mReadLicenseBtn.setNextFocusLeftId(R.id.settings_followUs);

		mClearCacheBtn.setNextFocusDownId(R.id.settings_DisplayDesktop);
		mClearCacheBtn.setNextFocusRightId(R.id.settings_DisplayDesktop);
		mClearCacheBtn.setNextFocusForwardId(R.id.settings_DisplayDesktop);
		mClearCacheBtn.setNextFocusUpId(R.id.settings_readLicense);
		mClearCacheBtn.setNextFocusLeftId(R.id.settings_readLicense);

		mDisplayDesktop.setNextFocusDownId(R.id.settings_onscreen);
		mDisplayDesktop.setNextFocusRightId(R.id.settings_onscreen);
		mDisplayDesktop.setNextFocusForwardId(R.id.settings_onscreen);
		mDisplayDesktop.setNextFocusUpId(R.id.settings_clearCache);
		mDisplayDesktop.setNextFocusLeftId(R.id.settings_clearCache);

		mCorporate_OnResSwitch.setNextFocusDownId(R.id.settings_onscreenWidth);
		mCorporate_OnResSwitch.setNextFocusRightId(R.id.settings_onscreenWidth);
		mCorporate_OnResSwitch.setNextFocusForwardId(R.id.settings_onscreenWidth);
		mCorporate_OnResSwitch.setNextFocusUpId(R.id.settings_DisplayDesktop);
		mCorporate_OnResSwitch.setNextFocusLeftId(R.id.settings_DisplayDesktop);

		mCorporate_Width.setNextFocusDownId(R.id.settings_onscreenHeight);
		mCorporate_Width.setNextFocusRightId(R.id.settings_onscreenHeight);
		mCorporate_Width.setNextFocusForwardId(R.id.settings_onscreenHeight);
		mCorporate_Width.setNextFocusUpId(R.id.settings_onscreen);
		mCorporate_Width.setNextFocusLeftId(R.id.settings_onscreen);

		mCorporate_Height.setNextFocusDownId(R.id.settings_forceDisplayBrightness);
		mCorporate_Height.setNextFocusRightId(R.id.settings_forceDisplayBrightness);
		mCorporate_Height.setNextFocusForwardId(R.id.settings_forceDisplayBrightness);
		mCorporate_Height.setNextFocusUpId(R.id.settings_onscreenWidth);
		mCorporate_Height.setNextFocusLeftId(R.id.settings_onscreenWidth);

		mCorporate_ForceBrightnessSwitch.setNextFocusDownId(R.id.settings_btBrightness0);
		mCorporate_ForceBrightnessSwitch.setNextFocusRightId(R.id.settings_btBrightness0);
		mCorporate_ForceBrightnessSwitch.setNextFocusForwardId(R.id.settings_btBrightness0);
		mCorporate_ForceBrightnessSwitch.setNextFocusUpId(R.id.settings_onscreenHeight);
		mCorporate_ForceBrightnessSwitch.setNextFocusLeftId(R.id.settings_onscreenHeight);

		mCorporate_Brightness0.setNextFocusDownId(R.id.settings_btBrightness25);
		mCorporate_Brightness0.setNextFocusRightId(R.id.settings_btBrightness25);
		mCorporate_Brightness0.setNextFocusForwardId(R.id.settings_btBrightness25);
		mCorporate_Brightness0.setNextFocusUpId(R.id.settings_forceDisplayBrightness);
		mCorporate_Brightness0.setNextFocusLeftId(R.id.settings_forceDisplayBrightness);

		mCorporate_Brightness25.setNextFocusDownId(R.id.settings_btBrightness50);
		mCorporate_Brightness25.setNextFocusRightId(R.id.settings_btBrightness50);
		mCorporate_Brightness25.setNextFocusForwardId(R.id.settings_btBrightness50);
		mCorporate_Brightness25.setNextFocusUpId(R.id.settings_btBrightness0);
		mCorporate_Brightness25.setNextFocusLeftId(R.id.settings_btBrightness0);

		mCorporate_Brightness50.setNextFocusDownId(R.id.settings_btBrightness75);
		mCorporate_Brightness50.setNextFocusRightId(R.id.settings_btBrightness75);
		mCorporate_Brightness50.setNextFocusForwardId(R.id.settings_btBrightness75);
		mCorporate_Brightness50.setNextFocusUpId(R.id.settings_btBrightness25);
		mCorporate_Brightness50.setNextFocusLeftId(R.id.settings_btBrightness25);

		mCorporate_Brightness75.setNextFocusDownId(R.id.settings_btBrightness100);
		mCorporate_Brightness75.setNextFocusRightId(R.id.settings_btBrightness100);
		mCorporate_Brightness75.setNextFocusForwardId(R.id.settings_btBrightness100);
		mCorporate_Brightness75.setNextFocusUpId(R.id.settings_btBrightness50);
		mCorporate_Brightness75.setNextFocusLeftId(R.id.settings_btBrightness50);

		mCorporate_Brightness100.setNextFocusDownId(R.id.settings_btBrightness100);
		mCorporate_Brightness100.setNextFocusRightId(R.id.settings_btBrightness100);
		mCorporate_Brightness100.setNextFocusForwardId(R.id.settings_btBrightness100);
		mCorporate_Brightness100.setNextFocusUpId(R.id.settings_btBrightness75);
		mCorporate_Brightness100.setNextFocusLeftId(R.id.settings_btBrightness75);
	}

	@Override
	public void onResume() {
		super.onResume();

		mState = SettingsState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));

		mBrightness = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getInt("brightness", -1) / 255.0f;
		mForceBrightness = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getBoolean("forceBrightness", false);
		mForceResolution = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getBoolean("forceResolution", false);
		mShowDesktop = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getBoolean("displayDesktop", false);
		mWidth = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getInt("customOnscreenWidth", 100);
		mHeight = getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).getInt("customOnscreenHeight", 100);

		if(((BenchmarkApplication)getActivity().getApplication()).mIsInitialized) {
			setCorporate(mCorporate);
			setupDisplayDesktop();
		}

		setupState();
		setupText();
		setupBrightness();
		mDeleteUser.setEnabled(false);
	}

	@Override
	public void onPause() {
		super.onPause();

		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putFloat("brightness", mBrightness);
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putBoolean("forceBrightness", mForceBrightness);
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putBoolean("forceResolution", mForceResolution);
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putBoolean("displayDesktop", mShowDesktop);
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenWidth", mWidth);
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenHeight", mHeight);
	}

	/**
	 * Manages the state change visibility.
	 */
	private void setupState() {

		if(mState == SettingsState.NORMAL) {
			mSettingsBack.setVisibility(View.VISIBLE);
			mFollowUsBack.setVisibility(View.GONE);
			mRegisterBack.setVisibility(View.GONE);
			mNavbarLeft.setVisibility(View.GONE);
			mNavbarRight.setVisibility(View.INVISIBLE);

		} else if(mState == SettingsState.REGISTER) {
			mSettingsBack.setVisibility(View.GONE);
			mFollowUsBack.setVisibility(View.GONE);
			mRegisterBack.setVisibility(View.VISIBLE);
			mNavbarLeft.setVisibility(View.VISIBLE);
			mNavbarRight.setVisibility(View.INVISIBLE);

		} else if(mState == SettingsState.FOLLOW) {
			mSettingsBack.setVisibility(View.GONE);
			mFollowUsBack.setVisibility(View.VISIBLE);
			mRegisterBack.setVisibility(View.GONE);
			mNavbarLeft.setVisibility(View.VISIBLE);
			mNavbarRight.setVisibility(View.INVISIBLE);

		}
	}

	/**
	 * Setup state dependent texts
	 */
	protected void setupText() {
		if(mState == SettingsState.NORMAL) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), ""));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "TabSettings"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));

		} else if(mState == SettingsState.REGISTER) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "Register"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));

		} else if(mState == SettingsState.FOLLOW) {
			mNavbarLeft.setText(Localizator.getString(this.getActivity(), "Back"));
			mNavbarTitle.setText(Localizator.getString(this.getActivity(), "FollowUs"));
			mNavbarRight.setText(Localizator.getString(this.getActivity(), ""));

		}

		mWhatIsThisTitle.setText(Localizator.getString(getActivity(), "NetRequirementDialogTitle"));
		mWhatIsThis.setText(Localizator.getString(getActivity(), "NotRegisteredSectionBody"));
		mFollowUsBtn.setText(Localizator.getString(getActivity(), "FindOutMore"));
		mReadLicenseBtn.setText(Localizator.getString(getActivity(), "ReadLicense"));
		mClearCacheBtn.setText(Localizator.getString(getActivity(), "ClearHistoryDialogTitle"));

		mUserStatus.setText(Localizator.getString(getActivity(), "Login"));
		mUserName.setHint(Localizator.getString(getActivity(), "User"));
		mPass.setHint(Localizator.getString(getActivity(), "Password"));
		mWelcome.setText(Localizator.getString(getActivity(), "Welcome"));
		mLogin.setText(Localizator.getString(getActivity(), "Login"));
		mRegister.setText(Localizator.getString(getActivity(), "Register"));
		mDeleteUser.setText(Localizator.getString(getActivity(), "Delete account"));
		mDisplayDesktopTV.setText(Localizator.getString(getActivity(), "ShowDesktop"));

		mFollow_Info.setText(Localizator.getString(getActivity(), "FollowUsSectionBody"));
		mFollow_Website.setText(Localizator.getString(getActivity(), getString(R.string.app_website_title)));
		mFollow_Kishonti.setText(Localizator.getString(getActivity(), "kishonti.net"));
		mFollow_ContactUs.setText(Localizator.getString(getActivity(), "ContactUs"));

		mRegister_MainText.setText(Localizator.getString(getActivity(), "RegistrationForm"));
		mRegister_User.setHint(Localizator.getString(getActivity(), "RegisterLogin"));
		mRegister_Pass.setHint(Localizator.getString(getActivity(), "RegisterPassword"));
		mRegister_PassAgain.setHint(Localizator.getString(getActivity(), "RegisterPasswordAgain"));
		mRegister_Email.setHint(Localizator.getString(getActivity(), "RegisterEmail"));
		mRegister_ConfirmButton.setText(Localizator.getString(getActivity(), "Register"));

		mCorporate_Features.setText(Localizator.getString(getActivity(), "CorporateFeatures"));
		mCorporate_CommercialTitle.setText(Localizator.getString(getActivity(), "CorporateCommercialTitle"));
		mCorporate_Commercial.setText(Localizator.getString(getActivity(), "CorporateCommercial"));
		mCorporate_OnResText.setText(Localizator.getString(getActivity(), "SetOnscrResolution"));
		mCorporate_WidthText.setText(Localizator.getString(getActivity(), "Width"));
		mCorporate_Width.setHint(Localizator.getString(getActivity(), "Width"));
		mCorporate_HeightText.setText(Localizator.getString(getActivity(), "Height"));
		mCorporate_Height.setHint(Localizator.getString(getActivity(), "Height"));
		mCorporate_ForceBrightness.setText(Localizator.getString(getActivity(), "ForceDisplayBrightness"));
	}

	/**
	 * Changes the togglebuttons and brightness switch state.
	 */
	private void setupBrightness() {
		mCorporate_ForceBrightnessSwitch.changeState(mForceBrightness && mCorporate);
		mCorporate_Brightness0.setEnabled(mForceBrightness && mCorporate);
		mCorporate_Brightness25.setEnabled(mForceBrightness && mCorporate);
		mCorporate_Brightness50.setEnabled(mForceBrightness && mCorporate);
		mCorporate_Brightness75.setEnabled(mForceBrightness && mCorporate);
		mCorporate_Brightness100.setEnabled(mForceBrightness && mCorporate);

		mCorporate_Brightness0.setChecked(mBrightness < 0.25);
		mCorporate_Brightness25.setChecked(mBrightness >= 0.25 && mBrightness < 0.5);
		mCorporate_Brightness50.setChecked(mBrightness >= 0.5 && mBrightness < 0.75);
		mCorporate_Brightness75.setChecked(mBrightness >= 0.75 && mBrightness < 1);
		mCorporate_Brightness100.setChecked(mBrightness >= 1);
	}

	/**
	 * Changes the resolution controls state.
	 */
	private void setupResolution() {
		mCorporate_OnResSwitch.changeState(mForceResolution && mCorporate);
		mCorporate_Width.setEnabled(mForceResolution && mCorporate);
		mCorporate_Height.setEnabled(mForceResolution && mCorporate);

		mCorporate_Width.setText("" + mWidth);
		mCorporate_Height.setText("" + mHeight);
	}

	/**
	 * Changes the resolution controls state.
	 */
	private void setupDisplayDesktop() {
		mDisplayDesktop.changeState(mShowDesktop);
		BenchmarkApplication.getModel(getActivity()).RefreshTestData();
	}

	/**
	 * Sets the version of this page to corporate or community in default behavior.
	 * @param b True if you want corporate.
	 */
	public void setCorporate(boolean b) {
		mCorporate = b;

		mCorporate_Commercial.setVisibility(b ? View.GONE : View.VISIBLE);
		mCorporate_CommercialTitle.setVisibility(b ? View.GONE : View.VISIBLE);
		mCorporate_Features.setEnabled(b);
		mCorporate_ForceBrightness.setEnabled(b);
		mCorporate_HeightText.setEnabled(b);
		mCorporate_WidthText.setEnabled(b);
		mCorporate_OnResText.setEnabled(b);

		mReadLicenseBtn.setEnabled(!b);
		mUserStatus.setEnabled(!b);
		mUserName.setEnabled(!b);
		mPass.setEnabled(!b);
		mWelcome.setEnabled(!b);
		mWelcomeUser.setEnabled(!b);
		mLogin.setEnabled(!b);
		mRegister.setEnabled(!b);
		mDeleteUser.setEnabled(!b);

		mCorporate_ForceBrightnessSwitch.setEnabled(b);
		mCorporate_OnResSwitch.setEnabled(b);

		setupBrightness();
		setupResolution();
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
			if (mState == SettingsState.REGISTER || mState == SettingsState.FOLLOW) {
				hideKeyboard();
				AnimatePageChange(mState, SettingsState.NORMAL);
			}

		} else if(v.getId() == R.id.settings_register) {
			if (mState == SettingsState.NORMAL) {
				hideKeyboard();
				AnimatePageChange(mState, SettingsState.REGISTER);
			}

		} else if(v.getId() == R.id.settings_followUs) {
			if (mState == SettingsState.NORMAL) {
				hideKeyboard();
				AnimatePageChange(mState, SettingsState.FOLLOW);
			}

		} else if(v.getId() == R.id.settings_btBrightness0 ||
				v.getId() == R.id.settings_btBrightness25 ||
				v.getId() == R.id.settings_btBrightness50 ||
				v.getId() == R.id.settings_btBrightness75 ||
				v.getId() == R.id.settings_btBrightness100) {
			hideKeyboard();
			onBrightnessButtonClick(v.getId());

		} else if(v.getId() == R.id.followus_contactUs ||
				v.getId() == R.id.followus_facebook ||
				v.getId() == R.id.followus_googleplus ||
				v.getId() == R.id.followus_kishonti ||
				v.getId() == R.id.followus_linkedin ||
				v.getId() == R.id.followus_twitter ||
				v.getId() == R.id.followus_vimeo ||
				v.getId() == R.id.followus_website ||
				v.getId() == R.id.followus_youtube) {
			hideKeyboard();
			onLinkButtonClick(v.getId());

		} else if(v.getId() == R.id.settings_readLicense) {
			hideKeyboard();
			LicenseDialog f = (LicenseDialog) getFragmentManager().findFragmentByTag("LicenseFragment");
			if (f != null) {
				f.dismiss();
			}
			licenseDialog.setOnDialogHandler(this);
			licenseDialog.show(getFragmentManager(), "LicenseFragment");

		} else if(v.getId() == R.id.settings_clearCache) {
			hideKeyboard();
			CommonDialog dialog = new ClearResultDialog();
			dialog.setOnDialogHandler(this);
			dialog.show(getFragmentManager(), "clearResultFragment");
		}

	}

	/**
	 * Handles the link button presses
	 * @param id Id of the pressed button
	 */
	public void onLinkButtonClick(int id) {
		Intent browserIntent = null;

		if(id == R.id.followus_contactUs) {
			Intent i = new Intent(Intent.ACTION_SEND);
			i.putExtra(android.content.Intent.EXTRA_EMAIL, new String[]{getString(R.string.app_email_address)});
			browserIntent = Intent.createChooser(i, "Send email");
		} else if (id == R.id.followus_facebook) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://www.facebook.com/KishontiLtd"));
		} else if (id == R.id.followus_googleplus) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://plus.google.com/u/0/b/113561619535028544544/113561619535028544544"));
		} else if (id == R.id.followus_kishonti) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(getString(R.string.kishontiWebsiteURL)));
		} else if (id == R.id.followus_linkedin) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://www.linkedin.com/company/kishonti-informatics"));
		} else if (id == R.id.followus_twitter) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://twitter.com/KishontiI"));
		} else if (id == R.id.followus_vimeo) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://vimeo.com/user11934111"));
		} else if (id == R.id.followus_website) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(getString(R.string.app_website_url)));
		} else if (id == R.id.followus_youtube) {
			browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://www.youtube.com/user/KishontiLtd"));
		}

		if(browserIntent != null)
			startActivity(browserIntent);
	}

	/**
	 * Handles the brightness change request from a togglebutton.
	 * @param id Id of the ToggleButton
	 */
	public void onBrightnessButtonClick(int id) {
		if(id == R.id.settings_btBrightness0) {
			mBrightness = 0;
		} else if (id == R.id.settings_btBrightness25) {
			mBrightness = 0.25f;
		} else if (id == R.id.settings_btBrightness50) {
			mBrightness = 0.5f;
		} else if (id == R.id.settings_btBrightness75) {
			mBrightness = 0.75f;
		} else if (id == R.id.settings_btBrightness100) {
			mBrightness = 1;
		}
		setupBrightness();

		// to set a minimum brightness value to avoid keylock with zero brightness value
		int disc_brightness = (int)(255*mBrightness);
		if(disc_brightness < 255) disc_brightness++;

		// save for later use
		getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("brightness", disc_brightness).commit();
	}

	/**
	 * Handles the switch state changes
	 */
	@Override
	public void onStateChanged(View view, boolean state) {
		if(view.getId() == R.id.settings_forceDisplayBrightness) {
			mForceBrightness = state;
			getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE)
				.edit().putBoolean("forceBrightness", mForceBrightness).commit();
			setupBrightness();

		} else if(view.getId() == R.id.settings_onscreen) {
			mForceResolution = state;
			getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE)
				.edit().putBoolean("forceResolution", mForceResolution).commit();
			setupResolution();

		} else if (view.getId() == R.id.settings_DisplayDesktop) {
			mShowDesktop = state;
			getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE)
				.edit().putBoolean("displayDesktop", mShowDesktop).commit();
			setupDisplayDesktop();
		}

	}

	@Override
	public boolean onDialogClicked(DialogFragment sender, boolean result) {
		if (sender == licenseDialog) {
			if (!result) {
				InitializerApplication.instance.mInitStarted = false;
				InitializerApplication.instance.mIsInitialized = false;
				InitializerApplication.instance.getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE)
					.edit().putBoolean(EULATask.KEY_EULAACCEPTED, false).commit();

				getActivity().setResult(1);
				getActivity().finish();
			}
			return true;
		} else if(sender.getTag().equals("clearResultFragment")) {
			if(result) {
				BenchmarkApplication.getAppModel().SetFragmentState(
						ResultsFragment.class.toString(),
						ResultsFragment.ResultsState.BEST.name());
				BenchmarkApplication.getAppModel().SetFragmentParams(
						ResultsFragment.class.toString(),
						"");
				BenchmarkApplication.getAppModel().SetFragmentState(
						CompareFragment.class.toString(),
						CompareFragment.CompareState.COMPARE.name());
				BenchmarkApplication.getAppModel().SetFragmentParams(
						CompareFragment.class.toString(),
						"");
				BenchmarkApplication.getModel(getActivity()).clearAll(getActivity());
				getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit()
					.putStringSet("SyncExclude", new HashSet<String>())
					.putString("SyncExcludeVersion", "")
					.commit();

				File f = FileUtils.getFile(getActivity().getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE).getString(DetermineBigDataDirTask.KEY_BIGDATADIR, "") + "/date.txt");
				f.delete();
			}
			return true;
		}

		return false;
	}

	@Override
	public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
		if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_NEXT) {
			final InputMethodManager imm = (InputMethodManager) getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
			imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
			int value = -1;
			try {
				value = Integer.parseInt(((EditText) v).getText().toString());
			} catch (NumberFormatException e) {
				e.printStackTrace();
			}
			if (v.getId() == R.id.settings_onscreenWidth) {
				getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenWidth", value).commit();
				mWidth = value;
			} else if (v.getId() == R.id.settings_onscreenHeight) {
				getActivity().getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putInt("customOnscreenHeight", value).commit();
				mHeight = value;
			}
		}

		return true;
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
//		public SettingsState fromState;
		public SettingsState toState;

		public AnimationState(SettingsState from, SettingsState to) {
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
	protected void AnimatePageChange(final SettingsState fromState, final SettingsState toState) {
		if(mAnimState == null) {

			if(fromState == SettingsState.NORMAL && toState == SettingsState.FOLLOW) {
				mAnimState = new AnimationState(fromState, toState);
				mSettingsBack.setVisibility(View.VISIBLE);
				mFollowUsBack.setVisibility(View.VISIBLE);

				mSettingsBack.startAnimation(mAnim_PageLeftOut);
				mFollowUsBack.startAnimation(mAnim_PageRightIn);

			} else if(fromState == SettingsState.NORMAL && toState == SettingsState.REGISTER) {
				mAnimState = new AnimationState(fromState, toState);
				mSettingsBack.setVisibility(View.VISIBLE);
				mRegisterBack.setVisibility(View.VISIBLE);

				mSettingsBack.startAnimation(mAnim_PageLeftOut);
				mRegisterBack.startAnimation(mAnim_PageRightIn);

			} else if(fromState == SettingsState.REGISTER && toState == SettingsState.NORMAL) {
				mAnimState = new AnimationState(fromState, toState);
				mRegisterBack.setVisibility(View.VISIBLE);
				mSettingsBack.setVisibility(View.VISIBLE);

				mSettingsBack.startAnimation(mAnim_PageLeftIn);
				mRegisterBack.startAnimation(mAnim_PageRightOut);

			} else if(fromState == SettingsState.FOLLOW && toState == SettingsState.NORMAL) {
				mAnimState = new AnimationState(fromState, toState);
				mFollowUsBack.setVisibility(View.VISIBLE);
				mSettingsBack.setVisibility(View.VISIBLE);

				mSettingsBack.startAnimation(mAnim_PageLeftIn);
				mFollowUsBack.startAnimation(mAnim_PageRightOut);
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
		if(mState == SettingsState.FOLLOW) {
			AnimatePageChange(mState, SettingsState.NORMAL);
			handled = true;

		} else if(mState == SettingsState.REGISTER) {
			AnimatePageChange(mState, SettingsState.NORMAL);
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
		mState = SettingsState.valueOf(BenchmarkApplication.getAppModel().getFragmentState(this));
	}

	@Override
	public String getDefaultStateString() {
		return SettingsState.NORMAL.name();
	}

}
