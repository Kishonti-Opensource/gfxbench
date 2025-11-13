/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VIRTUAL_DASHBOARD_H
#define VIRTUAL_DASHBOARD_H

#include "../../frameworks/testfw/testfw.h"
#include "ng/timer.h"
#include <string>

namespace tfw
{
	class Descriptor;
}

class KRL_Scene;

class virtual_dashboard : public tfw::TestBase
{
public:
	virtual_dashboard();
	
	~virtual_dashboard();

	virtual bool init ();
    virtual std::string result() { return result_; }

	virtual void run ();

private:
	int m_viewport_width;
	int m_viewport_height;
	int m_color_bpp;
	int m_depth_bpp;
	int m_fsaa;
	int m_test_time;
	bool m_is_endless;
	KRL_Scene *m_scene;

	long elapsedTime_;
    ng::cpu_timer loadTimer;
	int m_frames;

	bool parseConfig(const std::string &config, tfw::Descriptor &td);

	void storeResults();
    std::string result_;
};

#endif
