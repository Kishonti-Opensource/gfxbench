/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <algorithm>
#include "test_descriptor.h"
#include "xml_utils.h"
#include "platform.h"

#ifdef __APPLE__
#include <Metal/Metal.h>
#include <TargetConditionals.h>
#endif

#ifdef USE_ANY_GL
#include "opengl/ext.h"
#endif

#ifdef USE_METAL
#include "metal/mtl_types.h"
#endif

ScreenMode IntToScreenMode(const int value)
{
	switch(value)
	{
	case 0: return SMode_Onscreen;
	case 1: return SMode_Offscreen;
	case 2: return SMode_Hybrid;

	default: return SMode_Onscreen;
	}
}

TestDescriptor& TestDescriptor::SetScreenMode(const ScreenMode screen_mode)
{
	m_screen_mode = screen_mode;
	return *this;
}

ScreenMode TestDescriptor::GetScreenMode() const
{
	return m_screen_mode;
}

const char * ScreenModeToCStr(const ScreenMode mode)
{
	switch(mode)
	{
	case SMode_Onscreen: return "Onscreen";
	case SMode_Offscreen: return "Offscreen";
	case SMode_Hybrid: return "Hybrid";

	default: return "";
	}
}


TestDescriptor& TestDescriptor::SetName(const std::string& name)
{
	m_name = name;
	return *this;
}


TestDescriptor& TestDescriptor::SetPlayTime(const int playtime)
{
	m_play_time = playtime;
	return *this;
}


TestDescriptor& TestDescriptor::SetStartAnimationTime(const int startAnimationTime)
{
	m_start_animation_time = startAnimationTime;
	return *this;
}


TestDescriptor& TestDescriptor::SetColorBpp(const int colorbpp)
{
	m_color_bpp = colorbpp;
	return *this;
}


TestDescriptor& TestDescriptor::SetDepthBpp(const int depthbpp)
{
	m_depth_bpp = depthbpp;
	return *this;
}


TestDescriptor& TestDescriptor::SetFrameStepTime(const int timestep)
{
	m_frame_step_time = timestep;
	return *this;
}


TestDescriptor& TestDescriptor::SetFullscreen(const int fullscreen)
{
	m_is_fullscreen = fullscreen;
	return *this;
}


TestDescriptor& TestDescriptor::SetFSAA(const int samples)
{
	m_fsaa = samples;
	return *this;
}


TestDescriptor& TestDescriptor::SetWidth(const unsigned int width)
{
	m_viewport_width = width;
	return *this;
}


TestDescriptor& TestDescriptor::SetHeight(const unsigned int height)
{
	m_viewport_height = height;
	return *this;
}

TestDescriptor& TestDescriptor::SetVirtualResolution(const unsigned int virtres)
{
	m_virtual_resolution = virtres;
	return *this;
}

TestDescriptor& TestDescriptor::SetTestWidth(const unsigned int width)
{
	m_test_width = width;
	return *this;
}


TestDescriptor& TestDescriptor::SetTestHeight(const unsigned int height)
{
	m_test_height = height;
	return *this;
}
    
int TestDescriptor::GetTestWidth() const
{
    return m_test_width;
}

int TestDescriptor::GetTestHeight() const
{
    return m_test_height;
}

TestDescriptor& TestDescriptor::SetEngine(const std::string& engine)
{
	m_engine = engine;
	return *this;
}


TestDescriptor& TestDescriptor::SetSceneFile(const std::string& scenefile)
{
	m_scenefile = scenefile;
	return *this;
}


TestDescriptor& TestDescriptor::SetTextureType(const std::string& textureType)
{
	m_texture_type = textureType;
	return *this;
}


TestDescriptor& TestDescriptor::SetHybridRefreshMillisec(const int hybrid_refresh_msec)
{
	m_hybrid_refresh_msec = hybrid_refresh_msec;
	return *this;
}


TestDescriptor& TestDescriptor::SetZPrePass(const bool use_zprepass)
{
	m_zprepass = use_zprepass;
	return *this;
}


TestDescriptor& TestDescriptor::SetMinRamRequired(const int sizeInMB)
{
	m_min_ram_required = sizeInMB;
	return *this;
}

TestDescriptor& TestDescriptor::SetSingleFrame(const int singleFrame)
{
	m_single_frame = singleFrame;
	return *this;
}

TestDescriptor& TestDescriptor::SetForceHighp(const bool forceHighp)
{
	m_force_highp = forceHighp;
	return *this;
}

TestDescriptor& TestDescriptor::SetTessellationEnabled(const bool tessEnabled)
{
	m_tessellation_enabled = tessEnabled;
	return *this;
}

static bool defaultSort(int u, int v)
{
   return u < v;
}

TestDescriptor& TestDescriptor::SetScreenshotFrames(const std::string& frames)
{
	m_screenshot_frames.clear();
	m_screenshot_frames_list = frames;

	char *p = (char*)frames.c_str();
	
    char* retVal = strtok(p, ",");
    if(retVal)
	{
        while(p != 0)
	    {
		    int result2;
		    int result;

		    if(strchr(p, '-') != 0)
		    {
			    result = atoi( p);
			    result2 = atoi( strchr(p, '-') + 1);
		    }
		    else
		    {
			    result = result2 = atoi( p);
		    }
			
		    for(int i = result;i < result2 + 1; i++)
		    {
			    m_screenshot_frames.push_back(i);
		    }
		    p = strtok(0, ",");
	    }
    }

	std::sort(m_screenshot_frames.begin(), m_screenshot_frames.end(), defaultSort);

	return *this;
}


TestDescriptor& TestDescriptor::SetParticleBufferSaveFrames(const std::string& frames)
{
	ParseFrameList(m_particle_buffer_save_frames, frames);
	return *this;
}


void TestDescriptor::ParseFrameList(std::vector<int> &frame_list, const std::string &frame_list_str)
{
	frame_list.clear();

	char *p = (char*)frame_list_str.c_str();

	char* retVal = strtok(p, ",");
	if (retVal)
	{
		while (p != 0)
		{
			int result2;
			int result;

			if (strchr(p, '-') != 0)
			{
				result = atoi(p);
				result2 = atoi(strchr(p, '-') + 1);
			}
			else
			{
				result = result2 = atoi(p);
			}

			for (int i = result; i < result2 + 1; i++)
			{
				frame_list.push_back(i);
			}
			p = strtok(0, ",");
		}
	}

	std::sort(frame_list.begin(), frame_list.end(), defaultSort);
}


TestDescriptor& TestDescriptor::SetDebugBattery(const bool debug_battery)
{
    m_is_debug_battery = debug_battery;
    return *this;
}

 TestDescriptor& TestDescriptor::SetWarmupFrames(const int warmup_frames)
 {
     m_warmup_frames = warmup_frames < 0 ? 0 : warmup_frames;
     return *this;
 }

 TestDescriptor& TestDescriptor::SetAdaptationMode(const int adaptation_mode)
 {
     m_adaptation_mode = adaptation_mode;
     return *this;
 }

 TestDescriptor& TestDescriptor::SetWorkgroupSizes(const std::string &workgroup_sizes)
 {
	 m_workgroup_sizes = workgroup_sizes;
	 return *this;
 }


 TestDescriptor& TestDescriptor::SetReportInterval(const int report_interval)
 {
	 m_report_interval = report_interval;
	 return *this;
 }


 TestDescriptor& TestDescriptor::SetReportFilename(const std::string &report_file)
 {
	 m_report_filename = report_file;
	 return *this;
 }


TestDescriptor::TestDescriptor()
{
	SetDefaults();
}


const int TestDescriptor::GetRedColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 5 : 8;
}


const int TestDescriptor::GetGreenColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 6 : 8;
}


const int TestDescriptor::GetBlueColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 5 : 8;
}


TestDescriptor& TestDescriptor::SetDefaults()
{
	m_screenshot_frames.clear();
	m_name.clear();
	m_engine.clear();
	m_is_endless = false;
	m_is_fullscreen = false;
	m_min_ram_required = -1;
	m_viewport_width = -1;
	m_viewport_height = -1;
	m_test_height = -1;
	m_test_width = -1;
    m_virtual_resolution = false;
	m_color_bpp = -1;
	m_depth_bpp = -1;
	m_fsaa = 0;
	m_frame_step_time = -1;
	m_play_time = -1;
	m_brightness = -1.0f;
	m_fps_limit = -1.0f;
	m_stencil_bpp = -1;
	m_single_frame = -1;
	m_max_rendered_frames = -1;
	m_start_animation_time = 0;
	m_texture_type.clear();
	m_screen_mode = SMode_Onscreen;
	m_hybrid_refresh_msec = 40;
	m_zprepass = false;
    m_disabledRenderBits = 0;
	m_qm_save_image = false;
    m_warmup_frames = 0;
	m_loop_count = 0;
	m_battery_charge_drop = 0;
	qm_compare_frame = -1;
	m_never_cancel = false;
	m_report_interval = -1;
	m_report_filename = "";
	return *this;
}

const int TestDescriptor::GetMinRamRequired() const
{
	return m_min_ram_required;
}

bool TestDescriptor::isFixedTime() const
{
	return m_frame_step_time > 0;
}

const std::string TestDescriptor::GetTextureType() const
{
#ifdef OPENGL_IMPLEMENTATION_FRAMECAPTURE
	return "ETC1to565";
#endif

	if( m_texture_type == "Auto")
	{
#ifdef HAVE_DX
		return "DXT1";
#elif defined USE_METAL

#if TARGET_OS_EMBEDDED
    if (m_scenefile.find("scene_trex.xml") != std::string::npos)
    {
        return "PVRTC4" ;
    }
    else if (m_scenefile.find("scene_3.xml") != std::string::npos)
    {
        return "ETC2" ;
    }
    else if (m_engine.find("compressed_fill") != std::string::npos)
    {
        return "PVRTC4" ;
    }
	else if (m_scenefile.find("scene_4.xml") != std::string::npos)
	{
		return "ASTC" ;
	}
    else
    {
        // the default is ETC2
        return "ETC2" ;
    }
#else // TARGET_OS_EMBEDDED
    auto device = MTLCreateSystemDefaultDevice();
    if (device != nullptr && [device supportsFamily:MTLGPUFamilyApple2] ) // Mac A12 GPU
    {
        if (m_scenefile.find("scene_trex.xml") != std::string::npos)
        {
            return "PVRTC4" ;
        }
        else if (m_scenefile.find("scene_3.xml") != std::string::npos)
        {
            return "ETC2" ;
        }
        else if (m_engine.find("compressed_fill") != std::string::npos)
        {
            return "PVRTC4" ;
        }
        else if (m_scenefile.find("scene_4.xml") != std::string::npos)
        {
            return "ASTC" ;
        }
        else
        {
            // the default is ETC2
            return "ETC2" ;
        }
    }
    else if (m_scenefile.find("scene_4.xml") != std::string::npos)
    {
		return "DXT5";
	}
	else
	{
		return "DXT1";
	}
#endif
    
#else
		if(GLB::g_extension != 0)
		{
            if (m_scenefile.find("scene_4.xml") != std::string::npos)
            {
                // Use ASTC on GLES if supported, DXT5 on desktop GL
                if (GLB::g_extension->isES())
                {
					 if (GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_astc_ldr))
					 {
						return "ASTC";
					 }
					 else
					 {
						return "ETC2";
					 }
                }
                else
                {
                    return "DXT5";
                }
            }

			if ((GLB::g_extension->hasFeature(GLB::GLBFEATURE_es3_compatibility) ||
				GLB::g_extension->hasExtension(GLB::GLBEXT_es3_compatibility)) &&
                GLB::g_extension->isES() ) //do not use ETC2 on desktop GL
			{
				return "ETC2";
			}
			else if (GLB::g_extension->hasExtension( GLB::GLBEXT_compressed_ETC1_RGB8_texture) && GLB::g_extension->isES())
			{
				return "ETC1";
			}
			else if( GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_pvrtc) && GLB::g_extension->isES())
			{
				return "PVRTC4";
			}
			else if( GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_s3tc))
			{
				return "DXT1";
			}
			else 
			{
				return "888";
			}
		}
		else
		{
			return m_texture_type;
		}
#endif
	}
	else
	{
		return m_texture_type;
	}
}


const bool TestDescriptor::ValidateTextureType() const
{
#if defined ENABLE_FRAME_CAPTURE
	return true;
#endif
#if defined HAVE_DX
	if( m_texture_type == "Auto" || m_texture_type == "DXT1" || m_texture_type=="888" || m_texture_type=="ETC1to888" )
	{
		return true;
	}
	else
	{
		return false;
	}
#elif defined USE_METAL
    
	std::string texture_type = GetTextureType();
#if TARGET_OS_IPHONE
    bool texture_valid = texture_type=="ETC2" || texture_type=="PVRTC4" || texture_type=="ETC1to888" ;
    
    if ( (texture_type == "ASTC") && MetalRender::isASTCSupported())
    {
        texture_valid = true ;
    }
    
#else
    bool texture_valid = false;
    auto device = MTLCreateSystemDefaultDevice();
    if (device != nullptr && [device supportsFamily:MTLGPUFamilyApple2] ) // Mac A12 GPU
    {
        texture_valid = texture_type=="DXT5" || texture_type=="DXT1" || texture_type=="888" || texture_type=="ETC1to888" || texture_type=="PVRTC4" || texture_type=="ASTC"|| texture_type=="ETC2";
    }
    else
    {
        texture_valid = texture_type=="DXT5" || texture_type=="DXT1" || texture_type=="888" || texture_type=="ETC1to888" ;
    }

#endif
    return texture_valid ;
    
#else
	if( m_texture_type == "ETC1" && !GLB::g_extension->hasExtension( GLB::GLBEXT_compressed_ETC1_RGB8_texture))
	{
		return false;
	}
	else if( m_texture_type == "PVRTC4" && !GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_pvrtc))
	{
		return false;
	}
	else if( m_texture_type == "DXT1" && !GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_s3tc))
	{
		return false;
	}
    else if (m_texture_type == "DXT5")
    {
        if (!GLB::g_extension->hasExtension(GLB::GLBEXT_texture_compression_s3tc))
        {
            return false;
        }

        if (m_scenefile.find("scene_4.xml") != std::string::npos)
        {
            // sRGB is required for Scene4
            return GLB::g_extension->hasExtension(GLB::GLBEXT_texture_sRGB) || GLB::g_extension->hasExtension(GLB::GLBEXT_sRGB_formats);
        }
        return true;
    }
    else if ( m_texture_type == "ASTC" && !GLB::g_extension->hasExtension( GLB::GLBEXT_texture_compression_astc_ldr))
    {
        return false;
    }

	return true;
#endif
}

