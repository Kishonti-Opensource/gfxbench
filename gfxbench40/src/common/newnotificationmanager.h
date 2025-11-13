/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NEWNOTIFICATIONMANAGER_H
#define NEWNOTIFICATIONMANAGER_H

#include "kcl_texture.h"

namespace GLB {
	class NewNotificationManager : KCL::TextureFactory {

	protected:
		KCL::Texture *m_texture;
	public:
		NewNotificationManager();
		virtual ~NewNotificationManager();
		void UpdateLogo(const char* textureName);
		void UpdateLogo(KCL::Image *image);
		virtual void ShowLogo(bool stretch, bool blend)=0;
		void ShowLoadingLogo();
		void ShowRunningLogo();
		static NewNotificationManager* NewInstance();
	};
}

#endif