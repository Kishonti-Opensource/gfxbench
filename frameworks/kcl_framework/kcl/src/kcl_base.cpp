/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_base.h>

#include <kcl_image.h>
#include <kcl_material.h>
#include <kcl_mesh.h>
#include <kcl_planarmap.h>
#include <kcl_light2.h>
#include <kcl_math3d.h>

bool KCL::zRangeZeroToOneGlobalOpt = false;

void KCL::Initialize(bool zRangeZeroToOne)
{
	Release();

	zRangeZeroToOneGlobalOpt = zRangeZeroToOne;
}

void KCL::Release()
{
}
