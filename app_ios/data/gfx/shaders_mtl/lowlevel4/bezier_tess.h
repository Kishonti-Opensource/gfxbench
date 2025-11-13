/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct _bezier_patch
{
	hfloat4x4 Px;
	hfloat4x4 Py;
	hfloat4x4 Pz;
};

struct TessUniforms
{
	hfloat4x4 mvp;
	hfloat4x4 mv;
	hfloat4x4 model;
	
	hfloat4 cam_near_far_pid_vpscale;
	hfloat4 frustum_planes[6];
	
	hfloat2 view_port_size;
	hfloat time;
	hfloat itercount;

	hfloat3 view_pos;
};

struct PatchInOut
{
	_bezier_patch bp;
	hfloat patch_wire_scale;
};
