/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef EXTENSION_H
#define EXTENSION_H

#include "platform.h"
#include "bitmask.h"
#include "graphics/graphicscontext.h"

namespace GLB
{

/// Symbolic constants for extensions.
typedef enum GLBExtId
{
	GLBEXT_blend_equation_separate = 0,
	GLBEXT_blend_func_separate,
	GLBEXT_blend_subtract,
	GLBEXT_texture_compression_astc_ldr,
	GLBEXT_byte_coordinates,
	GLBEXT_compressed_ETC1_RGB8_texture,
	GLBEXT_compressed_paletted_texture,
	GLBEXT_depth_texture,
	GLBEXT_draw_texture,
	GLBEXT_extended_matrix_palette,
	GLBEXT_fixed_point,
	GLBEXT_framebuffer_object,
	GLBEXT_matrix_get,
	GLBEXT_matrix_palette,
	GLBEXT_point_size_array,
	GLBEXT_point_sprite,
	GLBEXT_query_matrix,
	GLBEXT_read_format,
	GLBEXT_single_precision,
	GLBEXT_stencil_wrap,
	GLBEXT_texture_cube_map,
	GLBEXT_texture_env_crossbar,
	GLBEXT_texture_mirrored_repeat,
	GLBEXT_egl_image,
	GLBEXT_depth24,
	GLBEXT_depth32,
	GLBEXT_element_index_uint,
	GLBEXT_fbo_render_mipmap,
	GLBEXT_fragment_precision_high,
	GLBEXT_mapbuffer,
	GLBEXT_rgb8_rgba8,
	GLBEXT_stencil1,
	GLBEXT_stencil4,
	GLBEXT_stencil8,
	GLBEXT_texture_3D,
	GLBEXT_texture_float_linear,
	GLBEXT_texture_half_float_linear,
	GLBEXT_texture_float,
	GLBEXT_texture_half_float,
	GLBEXT_texture_npot,
	GLBEXT_vertex_half_float,
	GLBEXT_texture_filter_anisotropic,
	GLBEXT_texture_compression_pvrtc,
	GLBEXT_texture_compression_s3tc,
	GLBEXT_es3_compatibility,
	GLBEXT_texture_cube_map_array,
	GLBEXT_tessellation_shader,
	GLBEXT_geometry_shader,
	GLBEXT_extension_pack_es31a,
	GLBEXT_primitive_bounding_box,
	GLBEXT_color_buffer_float,
    GLBEXT_color_buffer_half_float,
    GLBEXT_texture_sRGB,
    GLBEXT_sRGB_formats,
    GLBEXT_debug,

	GLBEXT_NUMOFEXTENSIONS
} GLBExtId;

typedef enum GLBfeatures
{
    GLBFEATURE_sampler_object = 0,
    GLBFEATURE_invalidate_framebuffer,
    GLBFEATURE_es3_compatibility,
	GLBFEATURE_vertex_array_object,
    GLBFEATURE_num_of_features,
} GLBFeatureId;

/// Class that can be used to query and initialize extensions.
class Extension
{
public:
	Extension (GraphicsContext * const graphics_context);
	bool init ();
    void findSupportedFeatures();

    void disableExtension(const GLBExtId &id)
    {
	    m_ext_disabled.set(id);
    }
    void enableExtension(const GLBExtId &id)
    {
	    m_ext_disabled.unset(id);
    }
    bool hasExtension (const GLBExtId &id) const
    {
	    return m_ext[id] && !m_ext_disabled[id];
    }

    void disableFeature(const GLBFeatureId &id)
    {
	    m_features_disabled.set(id);
    }
    void enableFeature(const GLBFeatureId &id)
    {
	    m_features_disabled.unset(id);
    }
    bool hasFeature (const GLBFeatureId &id) const
    {
        return m_features[id] && !m_features_disabled[id];
    }
	bool isES() { return !m_graphics_context ? false : (m_graphics_context->type() == GraphicsContext::GLES); }

	GraphicsContext* const getGraphicsContext() const { return m_graphics_context ; }

	void dumpFeatureState() ;

private:
	Bitmask m_ext;
	Bitmask m_ext_disabled;

    Bitmask m_features;
    Bitmask m_features_disabled;

	GraphicsContext * const m_graphics_context ;
};

extern Extension *g_extension;
extern void GLBDebugLabel(GLenum identifier, GLuint name, GLsizei length, const char *label);

}

#endif
