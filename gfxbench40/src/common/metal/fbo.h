/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBO_H
#define FBO_H

#include "platform.h"
#include "../gfxbench/global_test_environment.h"
#include "fbo_enums.h"


namespace KCL { class Image; }

namespace GLB
{

	class Texture2D;

	/// Class that can be used to initialize use and query of framebuffer objects from Metal.
	class FBO {
	public:
		virtual ~FBO();
        
        static FBO* CreateFBO(const GlobalTestEnvironment* const gte, KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label) ;

		static void bind( FBO *fbo);
		static void InvalidateLastBound();
		static void ResetInternalState();
		static void SetGlobalFBO( FBO *fbo);
		static FBO* GetGlobalFBO();
		static void CreateGlobalFBO(const GlobalTestEnvironment* const gte);
		static void DeleteGlobalFBO();
        static FBO* GetLastBind() ;
        
		static KCL::uint32 GetScreenshotImage(KCL::Image& img);
		
        const KCL::uint32& getWidth () const
		{
			return m_width;
		}

		const KCL::uint32& getHeight () const
		{
			return m_height;
		}

	protected:
        std::string m_debug_label;
        const GlobalTestEnvironment* const m_gte ;
        
        FBO(const GlobalTestEnvironment* const gte);
        
		void Destroy();
        
        KCL::uint32 m_width;
        KCL::uint32 m_height;

	private:
		
		static FBO* m_originalGlobal;
		static FBO* m_currentGlobal;
		static FBO* m_lastBound;
	};

}

#endif
