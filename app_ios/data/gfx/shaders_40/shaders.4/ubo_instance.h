/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_bindings.h"

#if defined(INSTANCING) // only define the uniform block if it’s needed

    struct InstanceData
    {
        highp mat4 model;
        highp mat4 inv_model;
    };

    layout(std140, binding = INSTANCE_BINDING_SLOT) uniform UBlockInstance
    {
       InstanceData instance_data[MAX_INSTANCES];
    };
    uniform int instance_offset;

    #ifdef EDITOR_MODE
    layout(std140, binding = INSTANCE_SELECTED_BINDING_SLOT) uniform UBlockInstanceSelected
    {
       vec4 instance_selected[MAX_INSTANCES]; //.x is selected .yzw - padding
    };
    #endif

#endif