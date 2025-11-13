/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
highp float getLuminance(highp vec3 color)
{	
	 vec3 lum_weights = vec3(0.299, 0.587, 0.114);	
	//vec3 lum_weights = vec3(0.3,0.59,0.11);
	return dot(color, lum_weights);
}