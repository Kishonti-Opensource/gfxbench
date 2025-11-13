/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_VRAPI_DUMMY_H
#define GFXB_VRAPI_DUMMY_H

#include "gfxb_scene5_ovr.h"
#include "gfxb_5_ovr_tools.h"
#include <graphics/ovrgraphicscontext_dummy.h>

ovrTracking vrapi_GetPredictedTracking(ovrMobile* m, double t)
{
	ovrTracking tr;
	tr.Status = 0;
	tr.HeadPose.Pose.Position.x = 0.0f;
	tr.HeadPose.Pose.Position.y = 0.0f;
	tr.HeadPose.Pose.Position.z = 0.0f;
	KCL::Quaternion q;
	q.fromAngleAxis(0.0f, KCL::Vector3D(0.0f, 0.0f, 1.0f));
	tr.HeadPose.Pose.Orientation.x = q.x;
	tr.HeadPose.Pose.Orientation.y = q.y;
	tr.HeadPose.Pose.Orientation.z = q.z;
	tr.HeadPose.Pose.Orientation.w = q.w;

	return tr;
}

#endif