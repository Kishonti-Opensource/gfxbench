/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_SCENE_VERSION_H
#define KCL_SCENE_VERSION_H

namespace KCL // KCL - Kishonti Common Library
{

	enum SceneVersion
	{
		SV_VDB = 0,
		SV_25,
		SV_27,
		SV_30,
		SV_31,
		SV_TESS,
		SV_40,
		SV_41,
		SV_50,
		SV_ADAS,
		SV_50_OVR,
		SV_INVALID
	};

}

#endif // KCL_SCENE_VERSION_H