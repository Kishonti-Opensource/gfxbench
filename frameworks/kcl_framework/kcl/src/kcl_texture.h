/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_TEXTURE_H
#define KCL_TEXTURE_H

#include "ogg_decoder.h"
#include "kcl_image.h"
#include "kcl_math3d.h"

namespace KCL {

	typedef enum _TextureType
	{
		Texture_Unknown = 0,
		Texture_1D = 1,
		Texture_2D = 2,
		Texture_3D = 3,
		Texture_Cube = 4,
		Texture_Array = 5,
		Texture_Video = 6,
		Texture_CubeArray = 7
	} TextureType;

	typedef enum _TextureFilter
	{
		TextureFilter_Unknown     = 0,
		TextureFilter_Nearest     = 1,
		TextureFilter_Linear      = 2,

		// Use to disable MIP filtering
		TextureFilter_NotApplicable = 3
	} TextureFilter;

	typedef enum _TextureWrap
	{
		TextureWrap_Unknown = 0,
		TextureWrap_Clamp = 1,
		TextureWrap_Repeat = 2,
		TextureWrap_Mirror = 3
	} TextureWrap;

    typedef enum _TextureCreate
    {
        TC_None              = 0x0000,
        TC_Commit            = 0x0001,
        TC_NearestFilter     = 0x0002,
        TC_NoMipmap          = 0x0004,
        TC_Clamp             = 0x0008,
        TC_Flip              = 0x0010,
		TC_IsSRGB		     = 0x0020,
		TC_AnisotropicFilter = 0x0040,
    } TextureCreate;

	class KCL_API Texture
	{
    public:

        static KCL::ImageFormat decodeSource;
        static KCL::ImageFormat decodeTarget;

		static void SetDecodeDetails(KCL::ImageFormat source, KCL::ImageFormat target)
		{
			decodeSource = source;
            decodeTarget = target;
		}

	protected:
        std::string m_name;
		float m_videoTime;
		_ogg_decoder* m_video;
		const KCL::Image* m_image;
		TextureType m_type;
		int m_mipLevels;
		bool m_anisotropic_filter_enabled;
		TextureFilter m_minFilter;
		TextureFilter m_magFilter;
		TextureFilter m_mipFilter;
		TextureWrap m_wrapS;
		TextureWrap m_wrapT;
		TextureWrap m_wrapU;
		Vector3D	m_scale;
        bool m_releaseUponCommit;
        bool m_isCommitted;
		bool m_isSRGB;
        bool m_sRGB_values;
        KCL::uint32 m_width;
        KCL::uint32 m_height;
        

	public:

		Texture();
		Texture(const Image* image, bool releaseUponCommit = false);
		Texture(_ogg_decoder* video, bool releaseUponCommit = false);
		Texture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);

		virtual ~Texture(void);

		// Getters...

        bool isDataInSRGB() const { return m_sRGB_values; }

        inline bool isCommitted() { return m_isCommitted; }

        inline const std::string& getName() { return m_name; }

		static TextureType getType(ImageType imageType);

		inline TextureType getType() const	{ return m_type; }

		//inline const Image* getImage() const { return m_image; }
		
		inline int getMipLevels() const		{ return m_mipLevels; }
		
		inline Vector3D getScale() const	{ return m_scale; }

		inline TextureFilter getMinFilter() const	{ return m_minFilter; }
		
		inline TextureFilter getMagFilter() const	{ return m_magFilter; }
		
		inline TextureFilter getMipFilter() const	{ return m_mipFilter; }	

		inline TextureWrap getWrapS() const	{ return m_wrapS; }
		
		inline TextureWrap getWrapT() const	{ return m_wrapT; }
		
		inline TextureWrap getWrapU() const	{ return m_wrapU; }

		inline _ogg_decoder* getVideo() const { return m_video; }
        
        inline KCL::uint32 getWidth () const { return m_width; }
        
        inline KCL::uint32 getHeight () const { return m_height; }

		inline bool isSRGB() const { return m_isSRGB ; }

		inline bool isAnisotropicFilterEnabled() { return m_anisotropic_filter_enabled; }

		// Setters...

		inline void setMipLevels(int mipLevels)			{ m_mipLevels = mipLevels; }
		
		inline void setScale(const Vector3D &scale)		{ m_scale = scale; }
		
		inline void setScale(float scale)				{ m_scale = Vector3D(scale, scale, scale); }
		
		inline void setScale(float s, float t, float u)	{ m_scale = Vector3D(s, t, u); }

		inline void setMinFilter(TextureFilter filter)	{ m_minFilter = filter; }
		
		inline void setMagFilter(TextureFilter filter)	{ m_magFilter = filter; }
		
		inline void setMipFilter(TextureFilter filter)	{ m_mipFilter = filter; }

		inline void setWrapS(TextureWrap wrap)	{ m_wrapS = wrap; }
		
		inline void setWrapT(TextureWrap wrap)	{ m_wrapT = wrap; }
		
		inline void setWrapU(TextureWrap wrap)	{ m_wrapU = wrap; }

		inline void setAnisotropicFilterEnabled(bool value) { m_anisotropic_filter_enabled = value; }

		inline void setSRGB(bool isSRGB) { m_isSRGB = isSRGB; }

		/**
			Sets an image as input for the texture. After this call the image might still be needed for the commit() call.
		*/
		void setImage(const KCL::Image *image);

		/**
			Sets an image as input for the texture. After this call the image might still be needed for the commit() call.
		*/
		void setImage(const KCL::Image *image, TextureType type);

		// Platform-dependent methods

		/**
			Binds the texture to the specified texture slot.
			@slotId		identifier of the texture slot
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long bind(KCL::uint32 slotId) = 0;

		/**
			Creates platform-specific resources for the texture. After this call the previously loaded image can be deleted.
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long commit() = 0;

		/**
			Releases platform-specific resources of the texture.
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long release() = 0;

		/**
			Sets the video time to the specified value. The texture data is updated to contain the specified frame.
			@time		video time in seconds
			@return		implementation-specific error code. 0 if the texture was not updated, positive value for success, negative for failures.
		*/
		virtual long setVideoTime(float time) = 0;

		virtual KCL::uint32 textureObject() = 0;

	};

	class TextureFactory
	{
	public:
		virtual ~TextureFactory() {}
		virtual Texture *CreateTexture(const Image* img, bool releaseUponCommit = false) = 0;
		Texture* CreateAndSetup(TextureType type, const char* file, KCL::uint32 createFlags = TC_None);
	};

}

#endif

