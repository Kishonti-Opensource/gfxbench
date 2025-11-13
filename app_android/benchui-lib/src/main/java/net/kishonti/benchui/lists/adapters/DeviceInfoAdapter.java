/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.adapters;

import java.util.ArrayList;
import java.util.List;

import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.systeminfo.swig.DataFormatter;
import net.kishonti.systeminfo.swig.FormattedDeviceInfo;
import net.kishonti.systeminfo.swig.FormattedDeviceInfoVector;
import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.theme.Localizator;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.TextView;

public class DeviceInfoAdapter extends BaseAdapter implements ListAdapter {

	private Context mContext;
	private List<InfoItem> mInfoElementList;
	private InfoItem mBatteryItem;
	private Properties mProps;
	private FormattedDeviceInfoVector mDeviceInfoVector;
	
	public DeviceInfoAdapter(Context context, Properties props) {
		mContext = context;
		mProps = props;
		
		fillAdapterInfo();
	}
	
	/**
	 * Fills the adapters list from the properties list we receive from the dataformatter
	 */
	private void fillAdapterInfo() {
		final DataFormatter df = new DataFormatter();
		mInfoElementList = new ArrayList<InfoItem>();

		if (mProps != null) {
			df.SetProperties(mProps);

			mDeviceInfoVector = df.GetFormattedStream();
			for (int i = 0; i < mDeviceInfoVector.size(); i++) {
				
				FormattedDeviceInfo fdi = mDeviceInfoVector.get(i);
				InfoItem infoItem;
				if(fdi.Name().equals("Battery")) {
					if (fdi.Major().contains("BatteryCharging")) {
						String batt_string = Localizator.getString(mContext, "BatteryCharging");
						fdi.SetMajor(fdi.Major().replace("BatteryCharging", batt_string));
					} else if (fdi.Major().contains("BatteryUnplugged")) {
						String batt_string = Localizator.getString(mContext, "BatteryUnplugged");
						fdi.SetMajor(fdi.Major().replace("BatteryUnplugged", batt_string));
					} else if (fdi.Major().contains("BatteryPlugged")) {
						String batt_string = Localizator.getString(mContext, "BatteryPlugged");
						fdi.SetMajor(fdi.Major().replace("BatteryPlugged", batt_string));
					}
					mBatteryItem = new InfoItem(fdi);
					infoItem = mBatteryItem;
					
				} else {
					infoItem = new InfoItem(fdi);
				}

				if (fdi.Features() == null || fdi.Features().size() <= 0) {
					infoItem.mFeatureLayout = null;
					
				} else {
					LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
					infoItem.mFeatureLayout = new LinearLayout(mContext);
					infoItem.mFeatureLayout.setOrientation(LinearLayout.VERTICAL);
					long featureSize = fdi.Features().size();
					for (int h = 0; h < featureSize; h += 2) {
						if (featureSize > h + 1) {
							addLine(inflater, infoItem.mFeatureLayout, fdi.Features().get(h).GetName(), fdi.Features().get(h).GetValue(), fdi.Features().get(h + 1).GetName(),
									fdi.Features().get(h + 1).GetValue());
						} else {
							addLine(inflater, infoItem.mFeatureLayout, fdi.Features().get(h).GetName(), fdi.Features().get(h).GetValue(), null, false);
						}
					}
				}
				
				mInfoElementList.add(infoItem);
			}
		}
	}
	
	/**
	 * Refreshes the battery status when somebody says its changed.
	 */
	public void refreshBattery(double level, boolean charging, boolean plugged) {
		String statusString = charging ? "BatteryCharging" : (plugged ? "BatteryPlugged" : "BatteryUnplugged");
		mBatteryItem.mMajor = "" + (int)(level*100) + "%, " 
				+ Localizator.getString(mContext, statusString);
	}
	
	/**
	 * Inflates a new line of feature rows.
	 * @param inflater The inflater to use.
	 * @param rows_view The base LinearLayout which will store the line.
	 * @param first The first elements name.
	 * @param first_state The first elements state.
	 * @param second The second elements name.
	 * @param second_state The second elements state.
	 */
	private void addLine(LayoutInflater inflater, LinearLayout rows_view, String first, boolean first_state, String second, boolean second_state) {
		LinearLayout featureLine = (LinearLayout)inflater.inflate(R.layout.layout_featureline, null, false);
		LinearLayout.LayoutParams featureLineLayout = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
				LinearLayout.LayoutParams.WRAP_CONTENT);

		rows_view.addView(featureLine, featureLineLayout);

		featureLine.setOrientation(LinearLayout.HORIZONTAL);
		addItem(inflater, featureLine, first, first_state);
		if (second != null) {
			addItem(inflater, featureLine, second, second_state);
		}
	}
	
	/**
	 * Inflates a new feature cell.
	 * @param inflater The inflater to use.
	 * @param fl The FeatureLine to use inflate in. 
	 * @param key The name of the feature.
	 * @param first_state The state of the feature.
	 */
	private void addItem(LayoutInflater inflater, LinearLayout fl, String key, boolean first_state) {
		View featureItem = inflater.inflate(R.layout.layout_feature_item, null, false);
		LinearLayout.LayoutParams pp = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 1.0f);
		TextView name = (TextView) featureItem.findViewById(R.id.featureName);
		ImageView state = (ImageView) featureItem.findViewById(R.id.featureState);
		name.setText(key + ":");
		state.setImageResource(first_state == true ? R.drawable.dev_info_tick_yes : R.drawable.dev_info_tick_no);
		fl.addView(featureItem, pp);
	}

	@Override
	public int getCount() {
		return mInfoElementList.size();
	}

	@Override
	public Object getItem(int position) {
		return mInfoElementList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	// To be able to show dividers we need allitems true, but 
	// isEnabled(position) as the right value wonder why... (Stackowerflow)
	@Override
	public boolean areAllItemsEnabled() {
		return true;
	}
	
	@Override
	public boolean isEnabled(int position) {
		InfoItem item = (InfoItem)getItem(position);
		return item.mTitle.startsWith("3D API");
	}
	

	@Override
	public View getView(int position, View v, ViewGroup parent) {
		InfoItem item = (InfoItem)getItem(position);

		if (v == null)
			v = newView(mContext, item, parent);
		
		DeviceInfoCellContainer c = (DeviceInfoCellContainer)v.getTag();
		
		c.mIcon.setImageResource(item.mIcon);
		c.mTitle.setText(Localizator.getString(mContext, item.mTitle));
		c.mMajor.setText(item.mMajor);
		c.mMinor.setText(item.mMinor);
		
		c.mMajor.setVisibility(item.mMajor.isEmpty() ? View.GONE : View.VISIBLE);
		c.mMinor.setVisibility(item.mMinor.isEmpty() ? View.GONE : View.VISIBLE);
		
		c.mArrow.setVisibility(item.mTitle.startsWith("3D API") ? View.VISIBLE : View.GONE);
		
		v.setBackgroundColor(item.mTitle.equals("Device") ?
				mContext.getResources().getColor(R.color.list_header_background) :
				mContext.getResources().getColor(android.R.color.transparent));
		
		c.mTitle.setTextColor(item.mTitle.equals("Device") ?
				mContext.getResources().getColor(R.color.list_title_text) :
				mContext.getResources().getColor(R.color.text_title));
		
		if(c.mFeatureConatiner != null) {
			c.mTextContainer.removeView(c.mFeatureConatiner);
		}
		
		if(item.mFeatureLayout != null) {
			if(item.mFeatureLayout.getParent() != null) {
				((ViewGroup)item.mFeatureLayout.getParent()).removeView(item.mFeatureLayout);
			}
			
			c.mFeatureConatiner = item.mFeatureLayout;
			LinearLayout.LayoutParams layoutparams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
					LinearLayout.LayoutParams.WRAP_CONTENT);
			c.mTextContainer.addView(c.mFeatureConatiner, layoutparams);
		}

		return v;
	}
	
	public View newView(Context context, InfoItem item, ViewGroup parent) {
		if (item == null) return null;
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		DeviceInfoCellContainer c = new DeviceInfoCellContainer();
//		View v = inflater.inflate(R.layout.cell_deviceinfo, parent, false);
		View v = inflater.inflate(Utils.getLayoutId(context, "cell_deviceinfo"), parent, false);
		
		c.mIcon = (ImageView)v.findViewById(R.id.cell_deviceinfo_icon);
		c.mArrow = (ImageView)v.findViewById(R.id.cell_deviceinfo_arrow);
		c.mTitle = (TextView)v.findViewById(R.id.cell_deviceinfo_title);    
		c.mMajor = (TextView)v.findViewById(R.id.cell_deviceinfo_major);   
		c.mMinor = (TextView)v.findViewById(R.id.cell_deviceinfo_minor);
		c.mTextContainer = (LinearLayout)v.findViewById(R.id.cell_deviceinfo_textback);
		c.mFeatureConatiner = null;
		
		v.setTag(c);
		
		return v;
		
	}
	
	private class DeviceInfoCellContainer {
		public ImageView mIcon;
		public ImageView mArrow;
		public TextView mTitle;
		public TextView mMajor;
		public TextView mMinor;
		public LinearLayout mTextContainer;
		public LinearLayout mFeatureConatiner;
	}
	
	public class InfoItem {
		public int mIcon;
		public String mTitle;
		public String mMajor;
		public String mMinor;
		public LinearLayout mFeatureLayout;
		
		public InfoItem(FormattedDeviceInfo info) {
			mTitle = info.Name();
			mMajor = info.Major();
			mMinor = info.Minor();
			
			if(mTitle.equals("Device")) {
				mIcon = R.drawable.info_device;
			} else if(mTitle.equals("OS")) {
				mIcon = R.drawable.info_os_android;
			} else if(mTitle.equals("Display")) {
				mIcon = R.drawable.info_display;
			} else if(mTitle.equals("CPU")) {
				mIcon = R.drawable.info_cpu;
			} else if(mTitle.equals("GPU")) {
				mIcon = R.drawable.info_gpu;
			} else if(mTitle.startsWith("3D API")) {
				mIcon = R.drawable.info_threedapi;
			} else if(mTitle.equals("Memory")) {
				mIcon = R.drawable.info_memory;
			} else if(mTitle.equals("Storage")) {
				mIcon = R.drawable.info_storage;
			} else if(mTitle.equals("BackCamera")) {
				mIcon = R.drawable.info_back_camera;
			} else if(mTitle.equals("FrontCamera")) {
				mIcon = R.drawable.info_front_camera;
			} else if(mTitle.equals("Battery")) {
				mIcon = R.drawable.info_battery;
			} else if(mTitle.equals("Features")) {
				mIcon = R.drawable.info_features;
			} else {
				Log.i("InfoFragment", "Cannot find icon for: " + mTitle);
				mIcon = R.drawable.info_device;
			}
		}
	}
	
}
