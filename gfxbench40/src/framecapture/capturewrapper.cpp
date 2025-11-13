/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "capturewrapper.h"
#include "ng/json.h"
#include "kcl_base.h"

using namespace GLB;

extern void startPlayerInstance(std::string tName);
extern void startInit();
extern void endInit();
extern void startRender();
extern void endRender();
extern void endPlayerInstance();

// tells if a TestDescriptor is a valid FrameCapture descriptor
bool FrameCapture::isCapturing(const TestDescriptor &ts)
{
	ng::JsonValue jvalue;
	ng::Result r;
	jvalue.fromString(ts.GetExtraData().c_str(), r);
	if (!r.error())
	{
		ng::JsonValue jobj;
		jobj = jvalue["frames_to_save"];
		if (jobj.isArray())
		{
			for (unsigned int i = 0;i < jobj.size(); i++)
			{
				if (jobj[i].isNumber()) return true;
			}
		}
	}
	return false;
}

FrameCapture::FrameCapture(const TestDescriptor &ts,TestBase *wrappedTest):TestWrapper(ts,wrappedTest), warmup_frames (10), capture_test_name("cap"), frame_warmup_count(0), capture_VAO_enabled(-1)
{
	ng::JsonValue jvalue;
	ng::Result r;
	jvalue.fromString(ts.GetExtraData().c_str(), r);
	if (!r.error())
	{
		ng::JsonValue jobj;
		jobj = jvalue["frames_to_save"];
		if (jobj.isArray())
		{
			for (unsigned int i = 0;i < jobj.size(); i++)
			{
				if (jobj[i].isNumber()) frames_to_save.push((int)jobj[i].number());
			}
		}
		jobj = jvalue["warmup_frames"];
		if (jobj.isNumber()) warmup_frames = (int)jobj.number();
		jobj = jvalue["capture_test_name"];
		if (jobj.isString()) capture_test_name = jobj.string();
		jobj = jvalue["capture_vao_enabled"];
		if (jobj.isBoolean()) capture_VAO_enabled = jobj.boolean()?1:0;
	}
}

FrameCapture::~FrameCapture()
{
	endPlayerInstance();
}

KCL::KCL_Status FrameCapture::init0()
{
	if (!getWrappedTest())
	{
		return KCL::KCL_TESTERROR_NOERROR;
	}

	if (capture_VAO_enabled!=-1)
	{
		getWrappedTest()->OverrideVAOusage(capture_VAO_enabled > 0);
	}

	startPlayerInstance(capture_test_name);

	updateInternalSettings();
	startInit();
	KCL::KCL_Status t = getWrappedTest()->init0();
	endInit();
	updateWrapperSettings();
	SetRuntimeError(t);
	return t;
}

bool FrameCapture::animate(const int time)
{
	if (!getWrappedTest()) return false;
	if (frames_to_save.empty()) return false;
	if (frame_warmup_count<warmup_frames)
	{
		frame_warmup_count++;
	} else {
		frame_warmup_count=0;
		frames_to_save.pop();
	}
	if (frames_to_save.empty()) return false;

	updateInternalSettings();
	getWrappedTest()->IncFrameCounter();
	getWrappedTest()->animate(frames_to_save.front());
	updateWrapperSettings();
	return true;
}

bool FrameCapture::render0(const char* screenshotName)
{
	if (!getWrappedTest())
	{
		return false;
	}

	updateInternalSettings();
	if (frame_warmup_count==warmup_frames) startRender();
	getWrappedTest()->render0();
	if (frame_warmup_count==warmup_frames) endRender();
	updateWrapperSettings();

	return true;
}
