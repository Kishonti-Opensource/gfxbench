/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#define LP2_UNIFORMS_BFR_SLOT    0
#define LP2_LIGHTNING_BFR_SLOT   1
#define LP2_RENDER_BFR_SLOT      2

#define LP2_WORK_GROUP_SIZE		16


struct lightning_pass2_uniforms
{
    hfloat4x4 mvp;
    
    int current_lightning_count;
};

