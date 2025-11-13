/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_5.h"
#include "gfxb_scene5.h"
#include "scene5/gfxb_scene5_pipeline.h"
#include "components/input_component.h"

using namespace GFXB;

GFXB::SceneBase *Gfxb5::CreateScene()
{
	return new GFXB::Scene5Pipeline();
}


std::string Gfxb5::ChooseTextureType()
{
	const std::string &texture_type = GetTestDescriptor().GetTextureType();

	if (texture_type != "Auto")
	{
		return texture_type;
	}

	bool astc_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ASTC) > 0;
	bool etc2_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ETC2) > 0;
	bool dxt5_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_DXT5) > 0;

	switch (nglGetApi())
	{
	case NGL_OPENGL:
	case NGL_DIRECT3D_11:
	case NGL_DIRECT3D_12:
	case NGL_METAL_MACOS:
        if (astc_supported)
        {
            return "ASTC";
        }
		if (dxt5_supported)
		{
			return "DXT5";
		}
		return "888";

	case NGL_OPENGL_ES:
	case NGL_METAL_IOS:
	{
		if (astc_supported)
		{
			return "ASTC";
		}
		if (etc2_supported)
		{
			return "ETC2";
		}
		return "888";
	}
	case NGL_VULKAN:
	{
#ifndef ANDROID
		if (dxt5_supported)
		{
			return "DXT5";
		}
#endif
		if (astc_supported)
		{
			return "ASTC";
		}
		if (etc2_supported)
		{
			return "ETC2";
		}
		return "888";
	}

	default:
		return "888";
	}
}


void Gfxb5::HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs)
{
	SceneTestBase::HandleUserInput(input_component, frame_time_secs);

	GFXB::Scene5 *scene = (GFXB::Scene5*)GetScene();

	if (input_component->IsKeyPressed(85)) //U
	{
		scene->DebugFreezCamera(!scene->DebugHasFreezCamera());
	}
	if (input_component->IsKeyPressed(72)) //H
	{
		scene->SetForceHighp(!scene->ForceHighp());
	}
}

