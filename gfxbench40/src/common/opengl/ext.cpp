/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <ng/log.h>
#include "stdc.h"
#include "opengl/ext.h"
#include "opengl/misc2_opengl.h"
#include "string.h"
#include <string>
#include <climits>
#include <ng/log.h>

using namespace GLB;


GLB::Extension *GLB::g_extension = 0;



/// Struct that holds a short label of several OpenGL extensions.
typedef struct
{
	GLBExtId extId;
	const char *extName;
} GLBExtDescription;

typedef struct
{
    GLBFeatureId featureId;
    int minMajorVersionGL;
    int minMinorVersionGL;
    int minMajorVersionES;
    int minMinorVersionES;
	const char* featureName ;
} GLBFeatureRequirements;

static const GLBFeatureRequirements features[] =
{
    //Name, GL X.X, GLES Y.Y
    { GLBFEATURE_sampler_object, 3, 3, 3, 0, "sampler object"}, 
    { GLBFEATURE_invalidate_framebuffer, 4, 0, 3, 0, "invalidate framebuffer"},
	{ GLBFEATURE_es3_compatibility, INT_MAX, INT_MAX, 3, 0, "es3_compatibility" },	// ES2 is supported as an extension on desktop
	{ GLBFEATURE_vertex_array_object, 4, 0, 3, 0, "vertex_array_object"},
    { GLBFEATURE_num_of_features, 0, 0, 0, 0, "num_of_features" }
};

static const GLBExtDescription extensions[] =
{
	{ GLBEXT_blend_equation_separate, "blend_equation_separate" },
	{ GLBEXT_blend_func_separate, "blend_func_separate" },
	{ GLBEXT_blend_subtract, "blend_subtract" },
	{ GLBEXT_texture_compression_astc_ldr, "texture_compression_astc_ldr" },
	{ GLBEXT_byte_coordinates, "byte_coordinates" },
	{ GLBEXT_compressed_ETC1_RGB8_texture, "compressed_ETC1_RGB8_texture" },
	{ GLBEXT_compressed_paletted_texture, "compressed_paletted_texture" },
	{ GLBEXT_depth_texture, "depth_texture" },
	{ GLBEXT_draw_texture, "draw_texture" },
	{ GLBEXT_extended_matrix_palette, "extended_matrix_palette" },
	{ GLBEXT_fixed_point, "fixed_point" },
	{ GLBEXT_framebuffer_object, "framebuffer_object" },
	{ GLBEXT_matrix_get, "matrix_get" },
	{ GLBEXT_matrix_palette, "matrix_palette" },
	{ GLBEXT_point_size_array, "point_size_array" },
	{ GLBEXT_point_sprite, "point_sprite" },
	{ GLBEXT_query_matrix, "query_matrix" },
	{ GLBEXT_read_format, "read_format" },
	{ GLBEXT_single_precision, "single_precision" },
	{ GLBEXT_stencil_wrap, "stencil_wrap" },
	{ GLBEXT_texture_cube_map, "texture_cube_map" },
	{ GLBEXT_texture_env_crossbar, "texture_env_crossbar" },
	{ GLBEXT_texture_mirrored_repeat, "texture_mirrored_repeat" },
	{ GLBEXT_egl_image, "egl_image" },
	{ GLBEXT_depth24, "depth24" },
	{ GLBEXT_depth32, "depth32" },
	{ GLBEXT_element_index_uint, "element_index_uint" },
	{ GLBEXT_fbo_render_mipmap, "fbo_render_mipmap" },
	{ GLBEXT_fragment_precision_high, "fragment_precision_high" },
	{ GLBEXT_mapbuffer, "mapbuffer" },
	{ GLBEXT_rgb8_rgba8, "rgb8_rgba8" },
	{ GLBEXT_stencil1, "stencil1" },
	{ GLBEXT_stencil4, "stencil4" },
	{ GLBEXT_stencil8, "stencil8" },
	{ GLBEXT_texture_3D, "texture_3D" },
	{ GLBEXT_texture_float_linear, "texture_float_linear" },
	{ GLBEXT_texture_half_float_linear, "texture_half_float_linear" },
	{ GLBEXT_texture_float, "texture_float" },
	{ GLBEXT_texture_half_float, "texture_half_float" },
	{ GLBEXT_texture_npot, "texture_npot" },
	{ GLBEXT_vertex_half_float, "vertex_half_float" },
	{ GLBEXT_texture_filter_anisotropic, "texture_filter_anisotropic" },
	{ GLBEXT_texture_compression_pvrtc, "texture_compression_pvrtc" },
	{ GLBEXT_texture_compression_s3tc, "texture_compression_s3tc" },
	{ GLBEXT_texture_compression_s3tc, "texture_compression_dxt1" },
	{ GLBEXT_texture_compression_s3tc, "compressed_texture_s3tc" },
	{ GLBEXT_es3_compatibility, "ES3_compatibility" },
	{ GLBEXT_texture_cube_map_array, "texture_cube_map_array" },
	{ GLBEXT_tessellation_shader, "tessellation_shader" },
	{ GLBEXT_geometry_shader, "geometry_shader" },
	{ GLBEXT_extension_pack_es31a, "extension_pack_es31a" },
	{ GLBEXT_primitive_bounding_box, "primitive_bounding_box" },
	{ GLBEXT_color_buffer_float, "color_buffer_float" },
    { GLBEXT_color_buffer_half_float, "color_buffer_half_float" },
    { GLBEXT_texture_sRGB, "texture_sRGB" },
    { GLBEXT_sRGB_formats, "sRGB_formats" },
    { GLBEXT_debug, "debug" },
	{ GLBEXT_NUMOFEXTENSIONS, 0 }
};


static const char *prefixes[] =
{
	"GL_OES",
	"OES", // for webgl
	"GL_ARB",
	"ARB",
	"GL_EXT",
	"EXT", // for webgl
	"GL_IMG",
	"GL_KHR",
	"WEBGL",
	"MOZ_WEBGL",
	"WEBKIT_WEBGL",
	"GL_ANDROID",
    "GL_NV", // for GL_NV_sRGB_formats, debug purposes
	0,
};


Extension::Extension (GraphicsContext * const graphics_context) : m_graphics_context(graphics_context)
{
}


void Extension::findSupportedFeatures()
{
	if (!m_graphics_context)
	{
		return;
	}
    int glVersionMajor = m_graphics_context->versionMajor() ;
	int glVersionMinor = m_graphics_context->versionMinor() ;

    for(int i=0; features[i].featureId != GLBFEATURE_num_of_features; ++i)
    {
        if(isES())
        {
            if( (glVersionMajor > features[i].minMajorVersionES) || 
                ((glVersionMajor == features[i].minMajorVersionES) && (glVersionMinor >= features[i].minMinorVersionES) )
                )
            {
                m_features.set(features[i].featureId);
            }
        }
        else
        {
            if( (glVersionMajor > features[i].minMajorVersionGL) || 
                ((glVersionMajor == features[i].minMajorVersionGL) && (glVersionMinor >= features[i].minMinorVersionGL) )
                )
            {
                m_features.set(features[i].featureId);
            }
        }
    }

}

bool Extension::init()
{
#if defined (HAVE_DX) || defined(OPENGL_IMPLEMENTATION_NULL)
	m_ext.set(GLBEXT_texture_compression_s3tc);
	return false;
#else

    findSupportedFeatures();

	std::string extension_string;

#if defined HAVE_GLES3 || defined __glew_h__
	if( m_graphics_context && m_graphics_context->versionMajor() >= 3 && glGetStringi)
	{
		int numext = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numext);

		for( int i=0; i<numext; i++)
		{
			const char *s = (const char*)glGetStringi( GL_EXTENSIONS, i);
			if( s)
			{
				extension_string += std::string( (char*)s) + " ";
			}
		}
	}
#endif

	const char *str = 0;
	if (extension_string.empty())
	{
		// Fall back to deprecated function
		str = (const char *) glGetString (GL_EXTENSIONS);		
	}
	else
	{
		str = extension_string.c_str();
	}

	if (!str)
	{
		return false;
	}

	int n = 0;
	int j = 0;
	int found = 0;
	char extName[128];
	char buf[128];

	while (*str)
	{
		n = 0;
		while (*str != ' ' && *str && n < 128)
		{
			extName[n++] = *str++;
		}
		if (!str)
		{
			continue;
		}
		extName[n] = 0;
		for (int i = 0; extensions[i].extId != GLBEXT_NUMOFEXTENSIONS; i++)
		{
			j = 0;
			found = 0;
			while (prefixes[j])
			{
				sprintf (buf, "%s_%s", prefixes[j], extensions[i].extName);
#if defined PLATFORM_LINUX || defined __SYMBIAN32__
				if (strcasecmp (buf, extName)== 0) {
#elif defined _WIN32
				if (_stricmp (buf, extName)== 0) {
#elif defined WINCE
				if (_stricmp (buf, extName)== 0) {
#else
				if (strcasecmp (buf, extName)== 0) {					
#endif
					m_ext.set (extensions[i].extId);
					found = 1;
					break;
				}
				j++;
			}
			if (found)
			{
				break;
			}
		}


		while (*str == ' ' && *str)
		{
			str++;
		}
	}

 	if( hasExtension( GLB::GLBEXT_primitive_bounding_box))
	{
		if ( CheckPBBExtCompatibility() == false)
		{
			disableExtension( GLB::GLBEXT_primitive_bounding_box);
		}
	}

	return false;

#endif
}


void Extension::dumpFeatureState()
{
	int max_length = 0 ;
	for (int i = 0 ; i < GLBFEATURE_num_of_features; i++)
	{
		int length = strlen(features[i].featureName) ;
		if (length > max_length)
		{
			max_length = length ;
		}
	}

	char buffer[8192] ;
	for (int i = 0 ; i < GLBFEATURE_num_of_features; i++)
	{
		const char* state = (hasFeature(features[i].featureId))?"ENABLED":"disabled" ;
		sprintf(buffer,"feature %-*s: %s",max_length,features[i].featureName,state) ;
		NGLOG_INFO("%s", buffer) ;
	}
}