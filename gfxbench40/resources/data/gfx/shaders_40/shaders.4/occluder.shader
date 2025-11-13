/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"

#ifdef TYPE_vertex

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_TEXCOORD1_LOCATION) in vec2 in_texcoord1;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

void main()
{
	gl_Position = mvp * vec4(in_position, 1.0);
}

#endif

#ifdef TYPE_fragment

out vec4 frag_color;
void main()
{			
	frag_color = vec4( 1.0);
}

#endif
