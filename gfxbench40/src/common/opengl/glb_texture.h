/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_TEXTURE_H
#define GLB_TEXTURE_H

#include "kcl_texture.h"
#include "platform.h"
#include <map>

namespace GLB
{
	class GLBTexture : public KCL::Texture
	{
	protected:
		KCL::uint32 m_textureId;

	public:
		GLBTexture();
		GLBTexture(const KCL::Image* image, bool releaseUponCommit = false);
		GLBTexture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);
		virtual ~GLBTexture(void);

		virtual long bind(KCL::uint32 slotId) = 0;
		virtual long commit() = 0;
		virtual long release() = 0;

		inline KCL::uint32 textureObject()	{ return m_textureId; }

	protected:
		static GLenum getGlTextureType(KCL::TextureType type);
		static GLint getGlTextureFilter(KCL::TextureFilter filter, KCL::TextureFilter mipFilter);
		static GLint getGlTextureWrap(KCL::TextureWrap wrap);
		static void getGlTextureFormatES2(KCL::ImageFormat imgFormat, GLint& internalFormat, GLint& format, GLenum& type);
		static void getGlTextureFormatES3(KCL::ImageFormat imgFormat, bool srgb, GLint& internalFormat, GLint& format, GLenum& type);
        static bool isFormatSupported(KCL::ImageFormat imgFormat);
		static GLfloat getMaxAnisotropy();
	};

	class GLBTextureES2 : public GLBTexture
	{
    public:
		GLBTextureES2();
		GLBTextureES2(const KCL::Image* image, bool releaseUponCommit = false);
		GLBTextureES2(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);
		virtual ~GLBTextureES2(void);

		virtual long bind(KCL::uint32 slotId);
		virtual long commit();
		virtual long release();
		virtual long setVideoTime(float time);

	protected:
		virtual long commit1D();
		virtual long commit2D();
		virtual long commit3D();
		virtual long commitArray();
		virtual long commitCube();
		virtual long commitCubeArray();
		virtual long commitVideo();

        virtual void setupSampling();
        virtual void bindSampling(KCL::uint32 slotId);

        virtual void generateMipmap ();
        virtual void uploadMipmap (int format);
        virtual void commitETC( int internalformat); //TODO: can uploadMipmap replace this special case?
        virtual void commitASTC();
	};

    struct sampler
    {
        KCL::uint32 m_samplerID;
        KCL::uint32 m_refCount;

        sampler(KCL::uint32 samplerID);
    };

    struct samplingDesc
    {
        GLenum m_minFilter;
        GLenum m_magFilter;

        GLenum m_wrapS;
        GLenum m_wrapT;
        GLenum m_wrapR;

		GLfloat m_aniosotropy;

		std::string id;

        samplingDesc(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, GLenum wrapR, GLfloat aniosotropy);
        bool operator<(const GLB::samplingDesc &other) const;
    };


    class GLBTextureES3 : public GLBTextureES2
	{
    public:
        GLBTextureES3();
		GLBTextureES3(const KCL::Image* image, bool releaseUponCommit = false);
		GLBTextureES3(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);
		virtual ~GLBTextureES3(void);

		virtual long commit2D();
		virtual long commit3D();
		virtual long commitArray();
		virtual long commitCube();
		virtual long commitCubeArray();
		virtual long commitVideo();

		static void resetSamplerMap();

		virtual long release();
	protected:
        virtual void setupSampling();
        virtual void bindSampling(KCL::uint32 slotId);

		bool commitAsCompressedBlocks(KCL::ImageFormat format);

        std::map<samplingDesc, sampler>::iterator m_samplerIt;
	};

	class GLBTextureFactory : public KCL::TextureFactory
	{
	public:
		virtual GLBTexture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false);
	};
}

#endif

