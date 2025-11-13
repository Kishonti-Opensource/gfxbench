/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager.h"
#include "platform.h"

namespace GLB {

class NewNotificationManagerGL: public NewNotificationManager
{
	public:
		NewNotificationManagerGL(bool needsCore = false);
		virtual ~NewNotificationManagerGL();
		virtual void ShowLogo(bool stretch, bool blend);
	protected:
		virtual KCL::Texture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false);
		
	private:
		unsigned int m_program;
        bool m_needsCore;

		static const float m_vertices[8];
		static const unsigned short m_indices[4];

#if defined __glew_h__
		unsigned int m_vertex_buffer;
		unsigned int m_index_buffer;
		unsigned int m_vao;
#endif
};

}