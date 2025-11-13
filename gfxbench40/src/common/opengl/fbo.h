/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBO_H
#define FBO_H

#include "platform.h"
#include "fbo_enums.h"
#include <string>
namespace KCL { class Image; }

namespace GLB
{

	class Texture2D;

	/// Class that can be used to initialize use and query of framebuffer objects from OpenGL.
	class FBO {
	public:
		FBO();
		FBO(KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label);
		~FBO();

		static void bind( FBO *fbo);
		static void InvalidateLastBound();
		static void ResetInternalState();
		static void SetGlobalFBO( FBO *fbo);
		static FBO* GetGlobalFBO();
		static void CreateGlobalFBO();
		static void DeleteGlobalFBO();

		static void InvalidateGlobalDepthAttachment() ;

		//static void SetGlobalFBOName( uint32 name);
		static KCL::uint32 GetScreenshotImage(KCL::Image& img);
		const KCL::uint32& getName () const
		{
			return m_name;
		}

		Texture2D *getTexture () const
		{
			return m_texture;
		}

		const KCL::uint32& getDepth () const
		{
			return m_depth_texture;
		}

		const KCL::uint32& getWidth () const
		{
			return m_width;
		}

		const KCL::uint32& getHeight () const
		{
			return m_height;
		}

		KCL::uint32 getTextureName () const;

		const KCL::uint32& DepthRenderbuffer () const
		{
			return m_depth_renderbuffer;
		}

	protected:
		void Destroy();

	private:
		std::string m_debug_label;
		KCL::uint32 m_name;
		Texture2D *m_texture;
		KCL::uint32 m_depth_renderbuffer;
		KCL::uint32 m_depth_texture;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		static FBO* m_originalGlobal;
		static FBO* m_currentGlobal;
		static FBO* m_lastBound;
	};

}

#endif
