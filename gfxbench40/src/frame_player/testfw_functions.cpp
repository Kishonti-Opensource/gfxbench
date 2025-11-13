/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_base.h"
#include "kcl_os.h"
#include "kcl_io.h"
#include "test_base.h"

#ifndef HAVE_DX
#include "ext.h"
#include "glb_opengl_state_manager.h"
#include "fbo.h"
#endif


#include "test_base0.h"
#include "frameplayer_glb.h"

#include "frame_includes"

namespace GLB
{

bool CreateGlobals(const char* data, const char* datarw)
{
   	KCL::g_os = KCL::OS::CreateI("");

	KCL::AssetFile::SetDataPath(data);
	KCL::AssetFile::SetRWDataPath(datarw);
#if !defined HAVE_DX
	g_extension = new Extension;
	g_extension->init ();
#endif

#if defined HAVE_DX
	//DX::Initialize(); //TODO::
#else
	GLB::Initialize();
	FBO::CreateGlobalFBO();
	OpenGLStateManager::Reset();
#endif
	return true;
}

void DestroyGlobals()
{
#if defined HAVE_DX
	DX::Release();
#else
	FBO::DeleteGlobalFBO();
	GLB::Release();
	OpenGLStateManager::Reset();
	delete g_extension;
	g_extension = 0;
#endif
	KCL::OS::DestroyI(&KCL::g_os);
	KCL::g_os = 0;
}


#include "frame_create_test"

}//namespace GLB
