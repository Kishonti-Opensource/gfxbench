/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager.h"
#include "kcl_io.h"

namespace GLB {

	NewNotificationManager::NewNotificationManager():m_texture(0)
	{
	}

	NewNotificationManager::~NewNotificationManager()
	{
		delete m_texture;
	}

	void NewNotificationManager::ShowLoadingLogo()
	{
		UpdateLogo("common/loading_glb_square.png");
		if(m_texture != NULL)
		{
			ShowLogo(false, true);
		}
	}
	
	void NewNotificationManager::ShowRunningLogo()
	{
		UpdateLogo("common/running_glb_square.png");
		if (m_texture != NULL)
		{
			ShowLogo(false, true);
		}
	}

	void NewNotificationManager::UpdateLogo(KCL::Image *image)
	{
		KCL::Texture *texture = CreateTexture(image);
        texture->setMipFilter(KCL::TextureFilter_NotApplicable);
        texture->setWrapS(KCL::TextureWrap_Clamp);
        texture->setWrapT(KCL::TextureWrap_Clamp);
        texture->setWrapU(KCL::TextureWrap_Clamp);
		if (texture)
		{
			delete m_texture;
			m_texture = texture;
			m_texture->commit();
		}
	}

	void NewNotificationManager::UpdateLogo(const char* textureName)
	{
		const KCL::uint32 logoFlags = KCL::TC_Clamp | KCL::TC_NearestFilter | KCL::TC_NoMipmap | KCL::TC_Flip;
		try
		{
			KCL::Texture* texture = CreateAndSetup( KCL::Texture_2D, textureName, logoFlags);
			if (texture)
			{
				delete m_texture;
				m_texture = texture;
				m_texture->commit();
			}
			else
			{
				INFO("missing %s", textureName);
			}
		}
		catch(KCL::IOException &e)
		{
			INFO("%s", e.what());
		}
	}
}