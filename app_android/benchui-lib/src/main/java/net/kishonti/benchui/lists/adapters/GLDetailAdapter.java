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
import net.kishonti.systeminfo.swig.ApiDevice;
import net.kishonti.systeminfo.swig.ApiDeviceVector;
import net.kishonti.systeminfo.swig.ApiInfo;
import net.kishonti.systeminfo.swig.ApiPlatform;
import net.kishonti.systeminfo.swig.ApiPlatformVector;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListAdapter;
import android.widget.TextView;

public class GLDetailAdapter extends BaseAdapter implements ListAdapter {

	private Context mContext;
	final private List<GLInfoItem> mData = new ArrayList<GLInfoItem>();
	
	public GLDetailAdapter(Context context, ApiPlatformVector detailVector) {
		mContext = context;
		
		for (int i = 0; i < detailVector.size(); i++) {
			ApiPlatform platform = detailVector.get(i);
			
			for (int j = 0; j < platform.getDevices().size(); j++) {
				ApiDevice device = platform.getDevices().get(j);
				
				for(int k = 0; k < device.getInfos().size(); k++) {
					ApiInfo info = device.getInfos().get(k);
					mData.add(new GLInfoItem(info.getName(), info.getInfo()));
				}
			}
			
		}
	}

	@Override
	public int getCount() {
		return mData.size();
	}

	@Override
	public Object getItem(int position) {
		return mData.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	@Override
	public boolean areAllItemsEnabled() {
		return true;
	}
	
	@Override
	public boolean isEnabled(int position) {
		return false;
	}

	@Override
	public View getView(int position, View v, ViewGroup parent) {
		GLInfoItem item = (GLInfoItem)getItem(position);

		if (v == null)
			v = newView(mContext, item, parent);
		
		GLInfoCellContainer c = (GLInfoCellContainer)v.getTag();
		
		c.mName.setText(item.mKey);
		c.mDetail.setText(item.mValue);

		return v;
	}
	
	public View newView(Context context, GLInfoItem item, ViewGroup parent) {
		if (item == null) return null;
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		GLInfoCellContainer c = new GLInfoCellContainer();
//		View v = inflater.inflate(R.layout.cell_glinfo, parent, false);
		View v = inflater.inflate(Utils.getLayoutId(context, "cell_glinfo"), parent, false);
		
		c.mName = (TextView)v.findViewById(R.id.cell_glinfo_name);    
		c.mDetail = (TextView)v.findViewById(R.id.cell_glinfo_detail); 
		
		v.setTag(c);
		
		return v;
		
	}
	
	private class GLInfoCellContainer {
		public TextView mName;
		public TextView mDetail;
	}
	
	private class GLInfoItem {
		public GLInfoItem(String key, String value) {
			mKey = key;
			mValue = value;
		}

		public String mKey;
		public String mValue;
	}
	
}
