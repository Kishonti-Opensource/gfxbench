/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_IMAGE_H
#define GLB_IMAGE_H

#include "kcl_image.h"


#include "opengl/ext.h"
#include <string>


namespace GLB
{
	extern Extension *g_extension;

	class Image2D : public KCL::Image2D
	{
		friend class ImageCube;
	public:
		virtual ~Image2D ();

		/*override*/ void commit_debug_mipmap (bool repeatS = false, bool repeatT = false);
		/*override*/ void commit (bool repeatS = false, bool repeatT = false);

		static int needDecoding;

		enum  {
			DecodeDisabled,
			DecodeTo565,
			DecodeTo888
		};

		static void SetDecode(int val)
		{
			needDecoding = val;
		}

		bool isFormatSupported() const
		{
#if 0 // was FRAME_CAPTURE_DEFINE, but not used now
			switch (m_format)
			{
			case KCL::Image_LUMINANCE_L8:
			case KCL::Image_ALPHA_A8:
			case KCL::Image_LUMINANCE_ALPHA_LA88:
			case KCL::Image_RGB565:
			case KCL::Image_RGBA4444:
			case KCL::Image_RGBA5551:
			case KCL::Image_RGB888:
			case KCL::Image_RGBA8888:
				return true;
			default:
				return (needDecoding==DecodeDisabled);
			}
#endif
			switch (m_format)
			{
			case KCL::Image_LUMINANCE_L8:
			case KCL::Image_ALPHA_A8:
			case KCL::Image_LUMINANCE_ALPHA_LA88:
			case KCL::Image_RGB565:
			case KCL::Image_RGBA4444:
			case KCL::Image_RGBA5551:
			case KCL::Image_RGB888:
			case KCL::Image_RGBA8888:
				return true;
			case KCL::Image_DXT1:
			case KCL::Image_DXT3:
			case KCL::Image_DXT5:
				if(!g_extension)
				{
					return false;
				}
				return g_extension->hasExtension(GLBEXT_texture_compression_s3tc);
			case KCL::Image_PVRTC2:
			case KCL::Image_PVRTC4:
				if(!g_extension)
				{
					return false;
				}
				return g_extension->hasExtension(GLBEXT_texture_compression_pvrtc);
			case KCL::Image_RGBA_ASTC_8x8:
				if(!g_extension)
				{
					return false;
				}
				return g_extension->hasExtension(GLBEXT_texture_compression_astc_ldr);
			case KCL::Image_ETC1:
				if(!g_extension)
				{
					return false;
				}
				return g_extension->hasExtension(GLBEXT_compressed_ETC1_RGB8_texture);
			case KCL::Image_ETC2_RGBA8888:
			case KCL::Image_ETC2_RGB:
				{
					//TODO: isFormatSupported, is_context_es3
					bool is_context_es3 = true;

					if( is_context_es3)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			default:
				return false;
			}
		}

		Image2D ();
	protected:

		virtual void uploadMipmap (int format);
		virtual void generateMipmap ();
		void generateMipmap (int internalformat, int format, int type);

		void commitASTC ();
		void commitETC (int internal_format);
	};

	///Object for loading, or storing cube map images.
	class ImageCube : public KCL::ImageCube
	{
	public:
		/*override*/ void commit (bool repeatS = false, bool repeatT = false);

	protected:
		/*override*/ void commitETC1 (int face);
		/*override*/ void generateMipmap ();
	};

}//namespace GLB

#endif
