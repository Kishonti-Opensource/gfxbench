/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_5_H
#define GFXB_5_H

#include "common/gfxb_scene_test_base.h"

namespace GFXB
{
	class Gfxb5 : public SceneTestBase
	{
	protected:
		virtual SceneBase *CreateScene() override;
		virtual std::string ChooseTextureType() override;
		virtual void HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs) override;
	};

	template<class T>
	class GFXBenchA : public Gfxb5 {
	public:
		virtual SceneBase *CreateScene() override
		{
			return Gfxb5::CreateScene();
		}
	};

}

#endif
