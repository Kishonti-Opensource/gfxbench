/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CAPTUREWRAPPER_H
#define CAPTUREWRAPPER_H

#include "kcl_base.h"
#include "test_wrapper.h"
#include <queue>

class FrameCapture: public TestWrapper
{
public:
	FrameCapture(const TestDescriptor &ts,TestBase *wrappedTest);
	~FrameCapture();

	const char* getUom() const {
		return "";
	}
	inline float getScore () const { return 0; }

	KCL::KCL_Status init0 ();
	bool animate (const int time);
	bool render0 (const char* screenshotName = NULL);
	static bool isCapturing(const TestDescriptor &ts);
private:
	int frame_warmup_count;
	int warmup_frames;
	int capture_VAO_enabled;
	std::string capture_test_name;
	std::queue<int> frames_to_save;
};

#endif

