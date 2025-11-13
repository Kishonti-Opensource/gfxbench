/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
				        
#define LP1_UNIFORMS_BFR_SLOT    0
#define LP1_ENDPOINTS_BFR_SLOT   1
#define LP1_LIGHTNING_BFR_SLOT   2
#define LP1_LIGHTS_BFR_SLOT      3

#define LP1_NOISE_MAP_TEX_SLOT   0

#define LP1_WORK_GROUP_SIZE		16


struct lightning_pass1_uniforms
{
	hfloat4 bone_segments[28];
    
    hfloat  time ;
    unsigned int bone_segment_count ;
    hfloat  _pad1 ;
    hfloat  _pad2 ;
    
    hfloat3 actor_pos ;
    hfloat3 actor_right ;
};


