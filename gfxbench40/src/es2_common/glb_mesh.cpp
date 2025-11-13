/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_mesh.h"

KCL::Mesh3 *GLB::Mesh3Factory::Create(const char *name)
{
	return new GLB::Mesh3(name);
}
