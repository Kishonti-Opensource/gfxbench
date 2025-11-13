/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.util.ArrayList;
import java.util.List;

import net.kishonti.testfw.TfwActivity;
import net.kishonti.swig.Descriptor;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class TfwDemoActivity extends TfwActivity {

	protected static final int MSG_TYPE_RANGE    = 0x1100;
	protected static final int MSG_TYPE_CHECKBOX = 0x1110;

	private OnSeekBarChangeListener mSeekBarListener = new OnSeekBarChangeListener() {

		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
		}

		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
		}

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			int tag = getTag(seekBar);
			sendMessage(MSG_TYPE_RANGE, tag, progress);
		}
	};
	private OnCheckedChangeListener mCheckedChangeListener = new OnCheckedChangeListener() {

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			int tag = getTag(buttonView);
			sendMessage(MSG_TYPE_CHECKBOX, tag, isChecked ? 1 : 0);
		}
	};

	@Override
    protected List<SurfaceView> initContentView(List<Descriptor> descriptors) {
		int cnt = descriptors.size();
		if (cnt != 1) {
			throw new RuntimeException(this.getClass().getName() + " provides only one SurfaceView. " + cnt + " requested");
		}
        setContentView(R.layout.tfw_main);
        ViewGroup root = (ViewGroup) findViewById(R.id.tfw_root_frame);
        ViewGroup controls = (ViewGroup) root.findViewById(R.id.tfw_controls);
        if (controls != null) {
        	setCallbackForControlls(controls);
        }
        SurfaceView surface = (SurfaceView) findViewById(R.id.surfaceView);
        List<SurfaceView> surfaceViews = new ArrayList<SurfaceView>();
        surfaceViews.add(surface);
        return surfaceViews;
	}

	@Override
	public void onTestInitialized(TestRunner t) {
		super.onTestInitialized(t);
		ViewGroup root = (ViewGroup) findViewById(R.id.tfw_root_frame);
		View init = findViewById(R.id.tfw_init);
		root.removeView(init);
	}

	private void setCallbackForControlls(ViewGroup parent) {
	    for(int i = 0; i < parent.getChildCount(); ++i) {
	        View view = parent.getChildAt(i);
	        if(view instanceof SeekBar) {
	        	SeekBar sb = (SeekBar) view;
	        	sb.setOnSeekBarChangeListener(mSeekBarListener);
	        } else if(view instanceof CheckBox) {
	        	CheckBox cb = (CheckBox) view;
	        	cb.setOnCheckedChangeListener(mCheckedChangeListener);
	        } else if(view instanceof ViewGroup) {
	        	setCallbackForControlls((ViewGroup) view);
	        }
	    }
	}

	private int getTag(View v) {
		String tag = (String) v.getTag();
		int value = Integer.valueOf(tag);
		return value;
	}

	private void sendMessage(int type, int arg1, int arg2) {
		// TODO: sending messages always to the first test
		net.kishonti.swig.Message msg = new net.kishonti.swig.Message(type, arg1, arg2, 0);
		getMessageQueue(0).push_back(msg);
	}
}
